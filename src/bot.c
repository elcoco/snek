#include "bot.h"

void bot_init(struct Bot* bot, struct Game* game, uint32_t xsize, uint32_t ysize)
{
    bot->xsize = xsize;
    bot->ysize = ysize;

    bot->game = game;
}

enum Direction pos_to_dir(Pos x0, Pos y0, Pos x1, Pos y1)
{
    if (x0 == x1 && y0 > y1)
        return DIR_N;
    else if (x0 == x1 && y0 < y1)
        return DIR_S;
    else if (x0 < x1 && y0 == y1)
        return DIR_E;
    else if (x0 > x1 && y0 == y1)
        return DIR_W;
    else
        return DIR_NONE;
}

static void draw_bar(WINDOW* win, char* str)
{
    add_str(win, 0, 0, CGREEN, CDEFAULT, "%s", str);
}


void bot_run(struct Bot* bot, WINDOW* field_win, WINDOW* bar_win)
{
    // FIXME All references to curses should be removed from this module,
    //       Drawing should only happen by using callbacks
    //
    for (int i=0 ; ; i++) {
        struct Astar astar;

        // we can't use malloc so we need to init everything here
        struct Node grid[bot->xsize*bot->ysize];
        struct Node* openset[bot->xsize*bot->ysize];
        struct Node* closedset[bot->xsize*bot->ysize];

        astar_init(&astar, grid, openset, closedset, bot->xsize, bot->ysize);

        // disable drawing by uncommenting
        astar.draw_open_cb    = bot->draw_open_cb;
        astar.draw_closed_cb  = bot->draw_closed_cb;
        astar.draw_path_cb    = bot->draw_path_cb;
        //astar.draw_wall_cb    = bot->draw_wall_cb;
        //astar.draw_refresh_cb = bot->draw_refresh_cb;


        // find start and end points
        struct FoodItem* end = *bot->game->food.fhead;
        struct Seg* start = *bot->game->snake.stail;
        astar_set_points(&astar, start->xpos, start->ypos, end->xpos, end->ypos);

        // set snake body as wall in astar
        struct Seg* seg = *bot->game->snake.shead;
        while (seg != NULL) {
            struct Node* n = get_node(astar.grid, seg->xpos, seg->ypos, bot->xsize);
            n->is_wall = true;
            seg = seg->next;
        }

        enum ASResult res;

        // solve path using algorithm, use longest route as snake grows
        if (bot->game->snake.len < 100)
            res = astar_find_path(&astar, AS_SHORTEST);
        else
            res = astar_find_path(&astar, AS_LONGEST);

        if (res == AS_UNSOLVED) {
            show_msg("ASTAR UNSOLVABLE");
            break;
        }

        // get last node in found path from astar
        struct Node* n_end = get_node(astar.grid, end->xpos, end->ypos, bot->xsize);

        werase(field_win);

        // draw path
        astar_draw(&astar, n_end);

        struct Node* n_prev = get_node(astar.grid, start->xpos, start->ypos, bot->xsize);

        // find first node in path
        for (int gi=0 ; gi<=n_end->g ; gi++) {

            //while (n->g != 1)
            //    n = n->parent;
            struct Node* n = n_end;

            while (n->g != gi)
                n = n->parent;

            // find direction change from start and recommendation
            enum Direction dir = pos_to_dir(n_prev->x, n_prev->y, n->x, n->y);

            // apply move
            enum GameState gs = game_next(bot->game, dir);

            werase(field_win);
            //astar_draw(&astar, n_end);
            game_draw(bot->game);

            // draw bar
            char buf[256] = "";
            sprintf(buf, "Iteration: %d  snek_len: %d  score: %d, openset_len: %d, closedset_len: %d", i, bot->game->snake.len, bot->game->score, astar.openset.len, astar.closedset.len);
            draw_bar(bar_win, buf);
            wrefresh(bar_win);
            bot->draw_refresh_cb();

            usleep(2*1000);

            n_prev = n;
        }
    }
}
