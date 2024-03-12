#include <vector>
#include <random>
#include <cursesapp.h>
#include <cursesm.h>

enum FieldColor { COLOR_1 = 1, COLOR_2 = 2, COLOR_3 = 3, COLOR_4 = 4, COLOR_5 = 5 };
enum FieldState { EMPTY, MARKED, REMOVED, COLORED };

struct Field {
    FieldColor color;
    FieldState state = EMPTY;

    Field(FieldColor color) : color(color) {}

    Field() : color(COLOR_1) {}
};

class Board {
private:
    std::vector<std::vector<Field>> board{9, std::vector<Field>(9)};
    std::mt19937 random;
    std::uniform_int_distribution<uint8_t> random_int{1, 5};
    int points = 0;
    int tmp_points = 0;
    FieldColor last_color = COLOR_1;

public:

    Board() {
        random.seed(time(nullptr));
    }

    void randomize() {
        int i = 0;
        for(std::vector<Field>& row : board) {
            i++;
            int j = 0;
            for(Field& f : row) {
                j++;
                f.color = static_cast<FieldColor>(random_int(random));
                f.state = COLORED;
            }
        }
    }

    void demoStateLooose() {
        int i = 0;
        for(std::vector<Field>& row : board) {
            i++;
            int j = 0;
            for(Field& f : row) {
                j++;
                f.color = static_cast<FieldColor>((i+j) % 4 + 1);
                f.state = COLORED;
            }
        }
        getField(7, 4).color = COLOR_3;
    }

    void demoStateWin() {
        int j = 0;
        for(std::vector<Field>& row : board) {
            j++;
            for(Field& f : row) {
                f.color = static_cast<FieldColor>(j % 4 + 1);
                f.state = COLORED;
            }
        }
    }

    void click(int y, int x) {
        int marked = markNeighbors(y, x);
        if(marked != 0) {
            points += marked * (marked - 1);
            getField(y, x).state = REMOVED;
            last_color = getField(y, x).color;
        }
    }

    bool checkLost() {
        for(int i = 0; i < getY(); i++) {
            for(int j = 0; j < getX(); j++) {
                if(!isAlone(i, j)) {
                    return false;
                }
            }
        }
        return true;
    }

    bool checkWon() {
        for(int x = 0; x < getX(); x++) {
            if(!checkRow(x)) {
                return false;
            }
        }
        return true;
    }

    bool checkRow(int x) {
        for(int i = 0; i < getY(); i++) {
            if(get(i, x).state == COLORED) {
                return false;
            }
        }
        /* all empty */
        board.erase(board.begin() + x);
        if(x <= getX() / 2) {
            board.insert(board.begin(), std::vector<Field>(getY()));
        } else {
            board.insert(board.end(), std::vector<Field>(getY()));
        }
        return true;
    }

    bool simulateFall() {
        for(int i = 0; i < getY(); i++) {
            for(int j = 0; j < getX(); j++) {
                Field current = get(i, j);
                if(current.state == REMOVED) {
                    bool is_outside;
                    int n = 0;
                    while(get(i+n+1, j, is_outside).state != COLORED && !is_outside) {
                        n++;
                    }
                    for(int g = i+n; g >= 0; g--) {
                        getField(g, j).color = get(g-n-1, j).color;
                        getField(g, j).state = get(g-n-1, j).state;
                    }
                    for(int g = 0; g < n+1; g++) {
                        getField(g, j).state = EMPTY;
                    }
                    checkRow(j);
                    return true;
                }
            }
        }
        return false;
    }

    bool simulateRemoval() {
        for(int i = 0; i < getY(); i++) {
            for(int j = 0; j < getX(); j++) {
                Field current = get(i, j);
                if(current.state == REMOVED) {
                    /* make numbers go up becuase it looks cool */
                    tmp_points++;
                    if(get(i+1, j).state == MARKED) {
                        getField(i+1, j).state = REMOVED;
                         return true;
                    }
                    if(get(i-1, j).state == MARKED) {
                        getField(i-1, j).state = REMOVED;
                        return true;
                    }
                    if(get(i, j+1).state == MARKED) {
                        getField(i, j+1).state = REMOVED;
                        return true;
                    }
                    if(get(i, j-1).state == MARKED) {
                        getField(i, j-1).state = REMOVED;
                        return true;
                    }
                }
            }
        }
        /* set the real point number */
        tmp_points = points;
        return false;
    }

    Field get(int y, int x) {
        return getField(y, x);
    }

    Field get(int y, int x, bool& is_outside) {
        return getField(y, x, is_outside);
    }

    int getY() {
        return board.begin()->size();
    }

    int getX() {
        return board.size();
    }

    int getPoints() {
        return tmp_points;
    }

    FieldColor getLastColor() {
        return last_color;
    }

private:
    int markNeighbors(int y, int x) {
        int result;
        if((result = markNeighborsRecurse(y, x, get(y, x).color)) != 1) {
            return result;
        } else {
            /* reverse marking */
            getField(y, x).state = COLORED;
            return 0;
        }
    }

    int markNeighborsRecurse(int y, int x, FieldColor color) {
        if(get(y, x).state != COLORED) {
            return 0;
        }
        bool is_outside;
        if(get(y, x, is_outside).color == color) {
            if(is_outside) {
                return 0;
            }
            getField(y, x).state = MARKED;
            return markNeighborsRecurse(y-1, x, color)
                   + markNeighborsRecurse(y+1, x, color)
                   + markNeighborsRecurse(y, x+1, color)
                   + markNeighborsRecurse(y, x-1, color) + 1;
        } else {
            return 0;
        }
    }

    bool isAlone(int y, int x) {
        Field f = get(y, x);
        if(f.state != COLORED) {
            return true;
        }
        return !(sameColor(y-1, x, f.color) || sameColor(y+1, x, f.color)
            || sameColor(y, x-1, f.color) || sameColor(y, x+1, f.color));
    }

    bool sameColor(int y, int x, FieldColor color) {
        bool is_outside;
        return get(y, x, is_outside).state == COLORED && !is_outside && get(y, x).color == color;
    }

    Field& getField(int y, int x) {
        bool a;
        return getField(y, x, a);
    }

    Field& getField(int y, int x, bool& is_outside) {
        int y_new = std::min(std::max(0, y), getY()-1);
        int x_new = std::min(std::max(0, x), getX()-1);
        is_outside = y != y_new || x != x_new;
        return board[x_new][y_new];
    }
};

class BoardWindow : public NCursesColorWindow {

public:
    BoardWindow(int height, int width) : NCursesColorWindow(height + 2, width, 0, 0) {
        init_pair(1, COLOR_BLUE, -1);
        init_pair(2, COLOR_YELLOW, -1);
        init_pair(3, COLOR_RED, -1);
        init_pair(4, COLOR_GREEN, -1);
        init_pair(5, COLOR_WHITE, -1);
    }

    void render(Board& b, bool final=false) {
        for(int i = 0; i < b.getY(); i++) {
            for(int j = 0; j < b.getX(); j++) {
                attrset(COLOR_PAIR(b.get(i, j).color));
                switch(b.get(i, j).state) {
                    case COLORED:
                        addstr(i, j, "●");
                        break;
                    case EMPTY:
                        addstr(i, j, " ");
                        break;
                    case MARKED:
                        addstr(i, j, "o");
                        break;
                    case REMOVED:
                        addstr(i, j, "◈");
                        break;
                }
            }
        }
        attrset(COLOR_PAIR(b.getLastColor()));
        printw(height()-1, width() - 4, "%4d", b.getPoints());

        if(final) {
            if(b.checkWon()) {
                printw(height()-1, 0, "Won: ");
            } else {
                if(b.checkLost()) {
                    printw(height()-1, 0, "Lost: ");
                }
            }
        }
        refresh();
    }
};

class App : public NCursesApplication {

private:
    static constexpr int BOARD_WIDTH = 9;
    static constexpr int BOARD_HEIGHT = 9;

public:
    App() : NCursesApplication(true) {
    }

    virtual int run() {
        use_default_colors();
        set_escdelay(25);
        curs_set(0);

        noecho();
        Root_Window->keypad(true);
        mousemask(BUTTON1_PRESSED, NULL);


        BoardWindow board = BoardWindow(BOARD_HEIGHT, BOARD_WIDTH);
        moveToCenter(board);
        Root_Window->overwrite(board);

        Board b;
        b.randomize();
        //b.demoStateLooose();
        //b.demoStateWin();

        board.render(b, true);

        int ch;
        MEVENT mort;

        while(true) {
            ch = getch();

            if(ch == KEY_MOUSE) {
                getmouse(&mort);
                b.click(mort.y - board.begy(), mort.x - board.begx());
                board.render(b);
            }

            while(b.simulateRemoval()) {
                napms(50);
                board.render(b);
            }
            while(b.simulateFall()) {
                napms(50);
                board.render(b);
            }
            board.render(b, true);

            /* ESCAPE */
            if(ch == 27) {
                break;
            }
            if(ch == KEY_RESIZE) {
                moveToCenter(board);
                board.render(b);
            }
        }

        return EXIT_SUCCESS;
    }

    virtual chtype window_backgrounds() const {
        return A_BOLD;
    }

private:
    void moveToCenter(NCursesWindow& window) {
        int start_x = std::max((Root_Window->width() - BOARD_WIDTH) / 2, 0);
        int start_y = std::max((Root_Window->height() - BOARD_HEIGHT) / 2, 0);
        Root_Window->clear();
        window.mvwin(start_y, start_x);
        Root_Window->refresh();
    }

};

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");

    int res;

    try {
        App app;
        app.handleArgs(argc, argv);
        res = app();
        endwin();
    } catch (const NCursesException *e) {
        endwin();
        std::cerr << e->message << std::endl;
        res = e->errorno;
    } catch (const NCursesException &e) {
        endwin();
        std::cerr << e.message << std::endl;
        res = e.errorno;
    } catch (const std::exception &e) {
        endwin();
        std::cerr << "Exception: " << e.what() << std::endl;
        res = EXIT_FAILURE;
    }
    return res;
}