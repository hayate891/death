#include <check.h>

#define _TESTING
#include "death.c"

START_TEST (test_event_handler)
{
    int type;
    state in;
    XEvent event;
    Display *display = XOpenDisplay(NULL);

    /* ignore all events other than KeyPress and KeyRelease */
    for (type = 0; type <= LASTEvent; ++type) {
        if ((type == KeyPress) || (type == KeyRelease)) {
            continue;
        }
        event.type = type;
        for (in = splash; in <= dead; ++in) {
            fail_unless(event_handler(in, event) == in);
        }
    }


    /* pressing q from any state results in quit */
    event.type         = KeyPress;
    event.xkey.display = display;
    event.xkey.keycode = XKeysymToKeycode(display, XK_q);
    for (in = splash; in <= dead; ++in) {
        fail_unless(event_handler(in, event) == quit);
    }


    /* pressing up... */
    event.type         = KeyPress;
    event.xkey.display = display;
    event.xkey.keycode = XKeysymToKeycode(display, XK_Up);
    /* ... from any non-dead state results in playing_up */
    for (in = splash; in < dead; ++in) {
        fail_unless(event_handler(in, event) == playing_up);
    }
    /* ... but from dead, results in splash */
    fail_unless(event_handler(dead, event) == splash);


    /* pressing down... */
    event.type         = KeyPress;
    event.xkey.display = display;
    event.xkey.keycode = XKeysymToKeycode(display, XK_Down);
    /* ... from any non-dead state results in playing_down */
    for (in = splash; in < dead; ++in) {
        fail_unless(event_handler(in, event) == playing_down);
    }
    /* ... but from dead, results in splash */
    fail_unless(event_handler(dead, event) == splash);


    /* pressing any other key... */
    event.type         = KeyPress;
    event.xkey.display = display;
    event.xkey.keycode = XKeysymToKeycode(display, XK_a);
    /* ... is a noop when playing */
    for (in = playing_nil; in <= playing_down; ++in) {
        fail_unless(event_handler(in, event) == in);
    }
    /* ... splash -> playing_nil */
    fail_unless(event_handler(splash, event) == playing_nil);
    /* ... dead -> splash */
    fail_unless(event_handler(dead, event) == splash);

    /* releasing up... */
    event.type         = KeyRelease;
    event.xkey.display = display;
    event.xkey.keycode = XKeysymToKeycode(display, XK_Up);
    /* ... is ignored for everything but playing_up */
    for (in = splash; in <= dead; ++in) {
        if (in != playing_up) {
            fail_unless(event_handler(in, event) == in);
        }
    }
    /* ... playing_up -> playing_nil */
    fail_unless(event_handler(playing_up, event) == playing_nil);

    /* releasing down... */
    event.type         = KeyRelease;
    event.xkey.display = display;
    event.xkey.keycode = XKeysymToKeycode(display, XK_Down);
    /* ... is ignored for everything but playing_down */
    for (in = splash; in <= dead; ++in) {
        if (in != playing_down) {
            fail_unless(event_handler(in, event) == in);
        }
    }
    /* ... playing_down -> playing_nil */
    fail_unless(event_handler(playing_down, event) == playing_nil);
}
END_TEST

short basic_world_assertions(world in) {
    int x, y;
    for (x = 0; x < DIM; ++x) {
        for (y = 0; y < DIM; ++y) {
            short alive = world_cell_alive(in, x, y);
            short n = world_cell_living_neighbors(in, x, y);

            if (! ((alive == 0) || (alive == 1))) {
                return 0;
            }

            if ((n < 0) || (n > 8)) {
                return 0;
            }
        }
    }
    return 1;
}

START_TEST (test_world_new)
{
    basic_world_assertions(world_new());
}
END_TEST

world str_to_world(short width, char *in) {
    int x, y;
    world out;

    for (x = 0; x < DIM; ++x) {
        for (y = 0; y < DIM; ++y) {
            out.cells[x][y] = 0;
        }
    }

    y = 0;
    while (in[width * y]) {
        for (x = 0; x < width; ++x) {
            /* printf("%c", in[width * y + x]); */
            out.cells[x][y] = (in[width * y + x] != '_');
        }
        /* printf("\n"); */
        ++y;
    }
    return out;
}

short worlds_are_equal(world w1, world w2) {
    int x, y;
    for (x = 0; x < DIM; ++x) {
        for (y = 0; y < DIM; ++y) {
            if (world_cell_alive(w1, x, y) != world_cell_alive(w2, x, y)) {
                return 0;
            }
        }
    }
    return 1;
}

void print_world(world w, short width, short height) {
    int x, y;
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            printf("%c", world_cell_alive(w, x, y) ? 'O' : '_');
        }
        printf("\n");
    }
}

START_TEST (test_world_step_block)
{
    char *block =
        "____"
        "_OO_"
        "_OO_"
        "____";

    world w0 = str_to_world(4, block);
    world w1 = world_step(w0);
    fail_unless(worlds_are_equal(w0, w1));
}
END_TEST

START_TEST (test_world_step_beehive)
{
    char *beehive =
        "______"
        "__OO__"
        "_O__O_"
        "__OO__"
        "______";
    world w0 = str_to_world(6, beehive);
    world w1 = world_step(w0);

    fail_unless(worlds_are_equal(w0, w1));
}
END_TEST

START_TEST (test_world_step_blinker)
{
    char *blinker0 =
        "_____"
        "__O__"
        "__O__"
        "__O__"
        "_____";

    char *blinker1 =
        "_____"
        "_____"
        "_OOO_"
        "_____"
        "_____";

    world w0 = str_to_world(5, blinker0);
    world w1 = world_step(w0);
    world w2 = world_step(w1);

    fail_unless(worlds_are_equal(w1, str_to_world(5, blinker1)));
    fail_unless(worlds_are_equal(w2, w0));
}
END_TEST

START_TEST (test_world_step_glider)
{
    char *glider0 =
        "________"
        "__O_____"
        "___O____"
        "_OOO____"
        "________";

    char *glider1 =
        "________"
        "________"
        "_O_O____"
        "__OO____"
        "__O_____";

    char *glider2 =
        "________"
        "________"
        "___O____"
        "_O_O____"
        "__OO____";

    char *glider3 =
        "________"
        "________"
        "__O_____"
        "___OO___"
        "__OO____";

    char *glider4 =
        "________"
        "________"
        "___O____"
        "____O___"
        "__OOO___";

    world w0 = str_to_world(8, glider0);
    world w1 = world_step(w0);
    world w2 = world_step(w1);
    world w3 = world_step(w2);
    world w4 = world_step(w3);

    fail_unless(worlds_are_equal(w1, str_to_world(8, glider1)));
    fail_unless(worlds_are_equal(w2, str_to_world(8, glider2)));
    fail_unless(worlds_are_equal(w3, str_to_world(8, glider3)));
    fail_unless(worlds_are_equal(w4, str_to_world(8, glider4)));
}
END_TEST

int main(void) {
    TCase *tc;
    Suite *suite;
    SRunner *sr;
    int n_failed;
    
    tc = tcase_create("death");
    tcase_add_test(tc, test_event_handler);
    tcase_add_test(tc, test_world_new);
    tcase_add_test(tc, test_world_step_block);
    tcase_add_test(tc, test_world_step_beehive);
    tcase_add_test(tc, test_world_step_blinker);
    tcase_add_test(tc, test_world_step_glider);

    suite = suite_create("death");
    suite_add_tcase(suite, tc);

    sr = srunner_create(suite);

    srunner_run_all(sr, CK_NORMAL);
    n_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return n_failed ? 1 : 0;
}
