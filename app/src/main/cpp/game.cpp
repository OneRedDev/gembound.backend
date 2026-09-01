#include <jni.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <map>
#include <sstream>
#include <algorithm>

using namespace std;

const int H = 20;
const int W = 40;

vector<vector<string>> mine_map(H, vector<string>(W, "."));

int p_y = 2, p_x = 2;
int p_hp = 30, max_hp = 30;
int gems = 0;
int monsters_slain = 0;
bool has_amulet = false;
bool game_over = false;
bool won = false;
bool shop_open = false;

string weapon = "Pickaxe";

int p_atk = 5;
int s_atk = 6;
int p_cost = 3;
int s_cost = 4;

int armor_tier = 0;

string armor_names[] = {
    "None",
    "Steel",
    "Metal",
    "Gold",
    "Crysteel"
};

int armor_costs[] = {
    0, 4, 6, 9, 15
};

map<pair<int,int>, int> rock_hp;
map<pair<int,int>, int> monster_hp;

string log_msg = "Objective: Find the Amulet (&)!";

void build_map() {

    mine_map.assign(H, vector<string>(W, "."));

    rock_hp.clear();
    monster_hp.clear();

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {

            if (y == 0 || y == H - 1 ||
                x == 0 || x == W - 1) {

                mine_map[y][x] = "#";
            }

            else if (y == 5 || y == 14) {

                if (x != 10 && x != 30)
                    mine_map[y][x] = "#";
                else
                    mine_map[y][x] = "+";
            }

            else if (x == 15 || x == 25) {

                if (y != 3 && y != 10 && y != 17)
                    mine_map[y][x] = "#";
                else
                    mine_map[y][x] = "+";
            }
        }
    }

    mine_map[3][18] = "@";
    mine_map[8][6] = "@";
    mine_map[8][14] = "@";
    mine_map[12][22] = "@";
    mine_map[13][2] = "@";

    mine_map[2][20] = "M";
    mine_map[8][10] = "M";
    mine_map[14][17] = "M";

    mine_map[7][22] = "%";
    mine_map[13][35] = "%";
    mine_map[1][5] = "%";

    mine_map[15][38] = "&";

    mine_map[p_y][p_x] = "Y";
}

int calculate_score(bool victory) {

    int score = gems + (monsters_slain * 10);

    if (victory)
        score += 500;

    return score;
}

void monster_turn() {

    vector<pair<int,int>> monsters;

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {

            if (mine_map[y][x] == "M")
                monsters.push_back({y, x});
        }
    }

    for (auto m : monsters) {

        int my = m.first;
        int mx = m.second;

        int dy = p_y - my;
        int dx = p_x - mx;

        if (abs(dy) <= 6 && abs(dx) <= 6) {

            int sy = my;
            int sx = mx;

            if (dy > 0)
                sy++;
            else if (dy < 0)
                sy--;

            if (sy != my &&
                sy >= 0 && sy < H &&
                mine_map[sy][mx] == ".") {

                int hp = 15;

                if (monster_hp.count({my, mx}))
                    hp = monster_hp[{my, mx}];

                mine_map[my][mx] = ".";
                mine_map[sy][mx] = "M";

                monster_hp[{sy, mx}] = hp;
                monster_hp.erase({my, mx});
            }

            else {

                if (dx > 0)
                    sx++;
                else if (dx < 0)
                    sx--;

                if (sx != mx &&
                    sx >= 0 && sx < W &&
                    mine_map[my][sx] == ".") {

                    int hp = 15;

                    if (monster_hp.count({my, mx}))
                        hp = monster_hp[{my, mx}];

                    mine_map[my][mx] = ".";
                    mine_map[my][sx] = "M";

                    monster_hp[{my, sx}] = hp;
                    monster_hp.erase({my, mx});
                }
            }
        }
    }

    int adj_y[] = {
        p_y - 1,
        p_y + 1,
        p_y,
        p_y
    };

    int adj_x[] = {
        p_x,
        p_x,
        p_x - 1,
        p_x + 1
    };

    for (int i = 0; i < 4; i++) {

        int y = adj_y[i];
        int x = adj_x[i];

        if (y >= 0 && y < H &&
            x >= 0 && x < W &&
            mine_map[y][x] == "M") {

            int damage;

            if (armor_tier == 0)
                damage = 3;
            else if (armor_tier == 1)
                damage = 2;
            else if (armor_tier == 2 || armor_tier == 3)
                damage = 1;
            else
                damage = 0;

            p_hp -= damage;

            log_msg =
                "Monster hit you! -" +
                to_string(damage) +
                " HP.";
        }
    }

    if (p_hp <= 0) {
        game_over = true;
        won = false;
        log_msg = "PERISHED.";
    }
}

void player_action() {

    int adj_y[] = {
        p_y - 1,
        p_y + 1,
        p_y,
        p_y
    };

    int adj_x[] = {
        p_x,
        p_x,
        p_x - 1,
        p_x + 1
    };

    bool acted = false;

    for (int i = 0; i < 4; i++) {

        int y = adj_y[i];
        int x = adj_x[i];

        if (y < 0 || y >= H ||
            x < 0 || x >= W)
            continue;

        string target = mine_map[y][x];

        // ROCK
        if (target == "@") {

            if (weapon == "Pickaxe") {

                pair<int,int> rock = {y, x};

                if (!rock_hp.count(rock))
                    rock_hp[rock] = 10;

                rock_hp[rock] -= p_atk;

                if (rock_hp[rock] > 0) {

                    log_msg =
                        "Rock HP: " +
                        to_string(rock_hp[rock]) +
                        "/10";
                }

                else {

                    int roll = rand() % 100;

                    if (roll < 20) {

                        mine_map[y][x] = "$";
                        log_msg = "Chest found!";
                    }

                    else if (roll <= 48) {

                        mine_map[y][x] = "V";
                        log_msg = "Vein dropped!";
                    }

                    else {

                        mine_map[y][x] = "*";
                        log_msg = "Gem dropped!";
                    }

                    rock_hp.erase(rock);
                }
            }

            else {

                log_msg = "Use Pickaxe (1).";
            }

            acted = true;
            break;
        }

        // MONSTER
        if (target == "M") {

            if (weapon == "Sword") {

                pair<int,int> monster = {y, x};

                if (!monster_hp.count(monster))
                    monster_hp[monster] = 15;

                monster_hp[monster] -= s_atk;

                if (monster_hp[monster] > 0) {

                    log_msg =
                        "Hit Enemy! HP: " +
                        to_string(monster_hp[monster]) +
                        "/15";
                }

                else {

                    mine_map[y][x] = ".";

                    monsters_slain++;

                    monster_hp.erase(monster);

                    log_msg = "Monster Slain!";
                }
            }

            else {

                log_msg = "Use Sword (2).";
            }

            acted = true;
            break;
        }
    }

    if (!acted)
        log_msg = "Nothing near you.";

    monster_turn();
}

void move_player(char command) {

    int ny = p_y;
    int nx = p_x;

    if (command == 'w')
        ny--;

    else if (command == 's')
        ny++;

    else if (command == 'a')
        nx--;

    else if (command == 'd')
        nx++;

    else
        return;

    if (ny < 0 || ny >= H ||
        nx < 0 || nx >= W) {

        log_msg = "Edge reached.";
        return;
    }

    string tile = mine_map[ny][nx];

    if (tile == "#" ||
        tile == "@" ||
        tile == "M" ||
        tile == "-" ||
        tile == "|") {

        log_msg = "Path blocked.";
        return;
    }

    if (tile == "*") {

        gems++;
        log_msg = "Got Gem!";
    }

    else if (tile == "V") {

        gems += 3;
        log_msg = "Got Vein!";
    }

    else if (tile == "%") {

        p_hp = min(max_hp, p_hp + 10);
        log_msg = "Healed 10 HP!";
    }

    else if (tile == "$") {

        int bonus = rand() % 4 + 2;

        gems += bonus;

        log_msg =
            "Chest open! +" +
            to_string(bonus) +
            " Gems.";
    }

    else if (tile == "&") {

        has_amulet = true;

        log_msg =
            "SECURED AMULET! Run!";
    }

    else {

        log_msg = "Moved.";
    }

    mine_map[p_y][p_x] = ".";

    p_x = nx;
    p_y = ny;

    mine_map[p_y][p_x] = "Y";

    if (p_y == 2 &&
        p_x == 2 &&
        has_amulet) {

        game_over = true;
        won = true;

        log_msg =
            "YOU ESCAPED THE FIRST MINE!";

        return;
    }

    monster_turn();
}

void shop_command(char command) {

    if (command == '4') {

        shop_open = false;
        log_msg = "Left shop.";

        return;
    }

    if (command == '1' &&
        gems >= p_cost) {

        gems -= p_cost;

        p_atk += 3;

        p_cost += 3;

        log_msg =
            "Upgraded Pickaxe!";
    }

    else if (command == '2' &&
             gems >= s_cost) {

        gems -= s_cost;

        s_atk += 3;

        s_cost += 4;

        log_msg =
            "Upgraded Sword!";
    }

    else if (command == '3' &&
             armor_tier < 4) {

        int next = armor_tier + 1;

        if (gems >= armor_costs[next]) {

            gems -= armor_costs[next];

            armor_tier = next;

            log_msg =
                "Bought " +
                armor_names[next] +
                " armor!";

            if (next == 3) {

                max_hp = 45;
                p_hp = 45;
            }
        }

        else {

            log_msg = "Not enough gems.";
        }
    }

    else {

        log_msg =
            "Not enough gems or invalid choice.";
    }
}

string render_game() {

    ostringstream out;

    out << "========== GEMBOUND ==========\n";

    out << "LOG: "
        << log_msg
        << "\n";

    out << "==============================\n";

    if (shop_open) {

        out << "\n";
        out << "=========== SHOP =============\n";

        out << "Gems: "
            << gems
            << "\n\n";

        out << "1. Pickaxe +3   Cost: "
            << p_cost
            << "\n";

        out << "2. Sword +3     Cost: "
            << s_cost
            << "\n";

        if (armor_tier < 4) {

            int next = armor_tier + 1;

            out << "3. "
                << armor_names[next]
                << "     Cost: "
                << armor_costs[next]
                << "\n";
        }

        out << "4. Exit\n";

        out << "==============================\n";

        return out.str();
    }

    if (game_over) {

        out << "\n";

        if (won)
            out << "YOU ESCAPED THE FIRST MINE!\n";
        else
            out << "PERISHED.\n";

        out << "\n";

        out << "Gems: "
            << gems
            << "\n";

        out << "Slain: "
            << monsters_slain
            << "\n";

        out << "SCORE: "
            << calculate_score(won)
            << "\n";

        out << "==============================\n";

        return out.str();
    }

    int camera = 5;

    int start_y =
        max(0, p_y - camera);

    int end_y =
        min(H, p_y + camera + 1);

    int start_x =
        max(0, p_x - camera);

    int end_x =
        min(W, p_x + camera + 1);

    for (int y = start_y;
         y < end_y;
         y++) {

        for (int x = start_x;
             x < end_x;
             x++) {

            out << mine_map[y][x]
                << ' ';
        }

        out << '\n';
    }

    out << "==============================\n";

    out << "HP: "
        << p_hp
        << "/"
        << max_hp
        << " | Gems: "
        << gems
        << "\n";

    out << "Amulet: ["
        << (has_amulet ? "SECURED" : "MISSING")
        << "]\n";

    out << "Armor: ["
        << armor_names[armor_tier]
        << "] | Slain: "
        << monsters_slain
        << "\n";

    out << "Weapon: ["
        << weapon
        << "]\n";

    out << "Pickaxe ATK: "
        << p_atk
        << "\n";

    out << "Sword ATK: "
        << s_atk
        << "\n";

    out << "\n";
    out << "W A S D = MOVE\n";
    out << "F = ACT\n";
    out << "1 = PICKAXE\n";
    out << "2 = SWORD\n";
    out << "SHOP = SHOP\n";

    return out.str();
}


// ============================================================
// ANDROID / JNI
// ============================================================

extern "C"
JNIEXPORT void JNICALL
Java_com_nextdeliph_gembound_MainActivity_nativeStart(
    JNIEnv*,
    jobject) {

    srand(1);

    p_y = 2;
    p_x = 2;

    p_hp = 30;
    max_hp = 30;

    gems = 0;

    monsters_slain = 0;

    has_amulet = false;

    game_over = false;

    won = false;

    shop_open = false;

    weapon = "Pickaxe";

    p_atk = 5;
    s_atk = 6;

    p_cost = 3;
    s_cost = 4;

    armor_tier = 0;

    log_msg =
        "Objective: Find the Amulet (&)!";

    build_map();
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_nextdeliph_gembound_MainActivity_nativeScreen(
    JNIEnv* env,
    jobject) {

    string screen =
        render_game();

    return env->NewStringUTF(
        screen.c_str()
    );
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_nextdeliph_gembound_MainActivity_nativeCommand(
    JNIEnv* env,
    jobject,
    jstring command) {

    const char* chars =
        env->GetStringUTFChars(
            command,
            nullptr
        );

    string input =
        chars ? chars : "";

    env->ReleaseStringUTFChars(
        command,
        chars
    );

    if (!input.empty() &&
        !game_over) {

        char command_char =
            input[0];

        if (shop_open) {

            shop_command(command_char);
        }

        else if (command_char == '1') {

            weapon = "Pickaxe";

            log_msg =
                "Ready: Pickaxe.";
        }

        else if (command_char == '2') {

            weapon = "Sword";

            log_msg =
                "Ready: Sword.";
        }

        else if (command_char == 'p') {

            shop_open = true;

            log_msg =
                "Shop opened.";
        }

        else if (command_char == 'f') {

            player_action();
        }

        else if (command_char == 'w' ||
                 command_char == 'a' ||
                 command_char == 's' ||
                 command_char == 'd') {

            move_player(command_char);
        }

        else if (command_char == 'q') {

            game_over = true;
            won = false;

            log_msg =
                "Abandoned.";
        }
    }

    string screen =
        render_game();

    return env->NewStringUTF(
        screen.c_str()
    );
}
