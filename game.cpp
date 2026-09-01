#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <map>

using namespace std;

const string C_PL = "\033[93m";
const string C_MN = "\033[31m";
const string C_HT = "\033[91m";
const string C_VN = "\033[96m";
const string C_AM = "\033[92m";
const string C_CH = "\033[33m";
const string C_FD = "\033[32m";
const string C_RS = "\033[0m";

const int H = 20;
const int W = 40;
vector<vector<string>> mine_map(
  H, vector<string>(W, ".")
);

int p_y = 2, p_x = 2;
int p_hp = 30, max_hp = 30;
int gems = 0;
int monsters_slain = 0;
bool has_amulet = false;

string weapon = "Pickaxe";
int p_atk = 5, s_atk = 6;
int p_cost = 3, s_cost = 4;

int armor_tier = 0;
string armor_names[] = {
  "None", "Steel", "Metal",
  "Gold", "Crysteel"
};
int armor_costs[] = {
  0, 4, 6, 9, 15
};

map<pair<int,int>, int> rock_hp;
map<pair<int,int>, int> monster_hp;

pair<int,int> flash_coord = {
  -1, -1
};
string log_msg = 
  "Objective: Find the Amulet (&)!";

void build_map() {
  for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
      if (y == 0 || y == H-1 || 
          x == 0 || x == W-1) {
        mine_map[y][x] = "#";
      }
      else if (y == 5 || y == 14) {
        if (x != 10 && x != 30) {
          mine_map[y][x] = "#";
        } else {
          mine_map[y][x] = "+";
        }
      }
      else if (x == 15 || x == 25) {
        if (y != 3 && y != 10 && 
            y != 17) {
          mine_map[y][x] = "#";
        } else {
          mine_map[y][x] = "+";
        }
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

int calculate_score(bool won) {
  int score = gems + 
              (monsters_slain * 10);
  if (won) score += 500;
  return score;
}

// Separate function for Monster Turn
void monster_turn() {
  // 1. Monsters move toward the player if close enough
  vector<pair<int,int>> mlist;
  for (int my = 0; my < H; my++) {
    for (int mx = 0; mx < W; mx++) {
      if (mine_map[my][mx] == "M") {
        mlist.push_back({my, mx});
      }
    }
  }
  for (auto m : mlist) {
    int my = m.first, mx = m.second;
    int dy = p_y - my, dx = p_x - mx;
    if (abs(dy) <= 6 && abs(dx) <= 6) {
      int sy = my, sx = mx;
      if (dy > 0) sy++; 
      else if (dy < 0) sy--;
      if (dx > 0) sx++; 
      else if (dx < 0) sx--;
      
      if (sy != my && mine_map[sy][mx] == ".") {
        int m_val = 15;
        if (monster_hp.find({my, mx}) != monster_hp.end()) {
          m_val = monster_hp[{my, mx}];
        }
        mine_map[my][mx] = "."; 
        mine_map[sy][mx] = "M";
        monster_hp[{sy, mx}] = m_val; 
        monster_hp.erase({my, mx});
      }
      else if (sx != mx && mine_map[my][sx] == ".") {
        int m_val = 15;
        if (monster_hp.find({my, mx}) != monster_hp.end()) {
          m_val = monster_hp[{my, mx}];
        }
        mine_map[my][mx] = "."; 
        mine_map[my][sx] = "M";
        monster_hp[{my, sx}] = m_val; 
        monster_hp.erase({my, mx});
      }
    }
  }

  // 2. Monsters attack the player if adjacent
  int adj_y[] = {p_y-1, p_y+1, p_y, p_y};
  int adj_x[] = {p_x, p_x, p_x-1, p_x+1};
  for (int i = 0; i < 4; i++) {
    int ny = adj_y[i];
    int nx = adj_x[i];
    if (ny >= 0 && ny < H && nx >= 0 && nx < W && mine_map[ny][nx] == "M") {
      string ca = armor_names[armor_tier];
      int dmg = (ca == "None") ? 3 : (ca == "Steel") ? 2 : (ca == "Metal" || ca == "Gold") ? 1 : 0;
      p_hp -= dmg; 
      log_msg += " Monster hit you! -" + to_string(dmg) + " HP.";
    }
  }
}

int main() {
  build_map();
  while (true) {
    if (p_hp <= 0) { 
      cout << "\nPERISHED. Score: " << calculate_score(false) << endl; 
      break; 
    }

    system("clear");
    cout << "LOG: " << log_msg << endl;
    cout << "=============================" << endl;
    
    int CAM = 5;
    int s_y = max(0, p_y - CAM);
    int e_y = min(H, p_y + CAM + 1);
    
    for (int y = s_y; y < e_y; y++) {
      string row_str = "";
      int s_x = max(0, p_x - CAM);
      int e_x = min(W, p_x + CAM + 1);
      
      for (int x = s_x; x < e_x; x++) {
        string tile = mine_map[y][x];
        if (tile == "Y") row_str += C_PL + tile + C_RS + " ";
        else if (tile == "M") {
          if (flash_coord.first == y && flash_coord.second == x) {
            row_str += C_HT + tile + C_RS + " ";
          } else {
            row_str += C_MN + tile + C_RS + " ";
          }
        }
        else if (tile == "V") row_str += C_VN + tile + C_RS + " ";
        else if (tile == "&") row_str += C_AM + tile + C_RS + " ";
        else if (tile == "$") row_str += C_CH + tile + C_RS + " ";
        else if (tile == "%") row_str += C_FD + tile + C_RS + " ";
        else row_str += tile + " ";
      }
      cout << row_str << endl;
    }
    
    cout << "=============================" << endl;
    cout << "HP: " << p_hp << "/" << max_hp << " | Gems: " << gems << endl;
    cout << "Amulet Status: [" << (has_amulet ? "SECURED" : "MISSING") << "]" << endl;
    cout << "Arm: [" << armor_names[armor_tier] << "] | Slain: " << monsters_slain << endl;
    cout << "Wpn: [" << weapon << "] (P:" << p_atk << " S:" << s_atk << ")" << endl;
    cout << "W/A/S/D=Move | F=Act | P=Shop | Q=Quit" << endl;
    cout << "=============================" << endl;
    
    cout << ": ";
    string raw_input;
    cin >> raw_input;
    if (raw_input.empty()) continue;
    char cmd = raw_input[0];

    flash_coord = {-1, -1};

    if (cmd == '1') { weapon = "Pickaxe"; log_msg = "Ready: Pickaxe."; continue; }
    if (cmd == '2') { weapon = "Sword"; log_msg = "Ready: Sword."; continue; }
    if (cmd == 'q') { cout << "\nAbandoned. Score: " << calculate_score(false) << endl; break; }

    if (cmd == 'p') {
      system("clear");
      cout << "=== SHOP ===" << endl;
      cout << "Gems: " << gems << endl;
      cout << "1. Pik (+3) - Cost: " << p_cost << endl;
      cout << "2. Srd (+3) - Cost: " << s_cost << endl;
      if (armor_tier < 4) {
        int nt = armor_tier + 1;
        cout << "3. " << armor_names[nt] << " - Cst: " << armor_costs[nt] << endl;
      }
      cout << "4. Exit\n? ";
      char sc; cin >> sc;
      if (sc == '1' && gems >= p_cost) { gems -= p_cost; p_atk += 3; p_cost += 3; log_msg = "SHOP: Upgraded Pickaxe!"; }
      else if (sc == '2' && gems >= s_cost) { gems -= s_cost; s_atk += 3; s_cost += 4; log_msg = "SHOP: Upgraded Sword!"; }
      else if (sc == '3' && armor_tier < 4) {
        int nt = armor_tier + 1;
        if (gems >= armor_costs[nt]) {
          gems -= armor_costs[nt]; armor_tier = nt; log_msg = "Bought armor!"; 
          if (armor_names[nt] == "Gold") { max_hp = 45; p_hp = 45; }
        }
      }
      continue;
    }

    if (cmd == 'f') {
      int adj_y[] = {p_y-1, p_y+1, p_y, p_y};
      int adj_x[] = {p_x, p_x, p_x-1, p_x+1};
      bool acted = false;
      
      for (int i = 0; i < 4; i++) {
        int ny = adj_y[i], nx = adj_x[i];
        if (ny >= 0 && ny < H && nx >= 0 && nx < W) {
          string tgt = mine_map[ny][nx];
          if (tgt == "@") {
            if (weapon == "Pickaxe") {
              pair<int,int> rc = {ny, nx};
              if (rock_hp.find(rc) == rock_hp.end()) rock_hp[rc] = 10;
              rock_hp[rc] -= p_atk;
              if (rock_hp[rc] > 0) log_msg = "Rock HP: " + to_string(rock_hp[rc]) + "/10";
              else {
                int roll = rand() % 100;
                if (roll < 20) { mine_map[ny][nx] = "$"; log_msg = "Chest found!"; }
                else if (roll <= 48) { mine_map[ny][nx] = "V"; log_msg = "Vein dropped!"; }
                else { mine_map[ny][nx] = "*"; log_msg = "Gem dropped!"; }
                rock_hp.erase(rc);
              }
            } else log_msg = "Use Pickaxe (1).";
            acted = true; break;
          }
          if (tgt == "M") {
            if (weapon == "Sword") {
              pair<int,int> mc = {ny, nx};
              if (monster_hp.find(mc) == monster_hp.end()) monster_hp[mc] = 15;
              monster_hp[mc] -= s_atk;
              if (monster_hp[mc] > 0) {
                log_msg = "Hit Enemy! HP: " + to_string(monster_hp[mc]) + "/15";
                flash_coord = mc;
              } else {
                mine_map[ny][nx] = "."; log_msg = "Monster Slain!";
                monsters_slain++; monster_hp.erase(mc);
              }
            } else log_msg = "Use Sword (2).";
            acted = true; break;
          }
        }
      }
      if (!acted) log_msg = "Nothing near you.";
      
      // TRIGGER MONSTER TURN AFTER PLAYER ATTACKS
      monster_turn();
      continue;
    }

    int ny = p_y, nx = p_x;
    if (cmd == 'w') ny--; else if (cmd == 's') ny++; else if (cmd == 'a') nx--; else if (cmd == 'd') nx++; else continue;
    if (ny < 0 || ny >= H || nx < 0 || nx >= W) { log_msg = "Edge reached."; continue; }
    
    string tile = mine_map[ny][nx];
    if (tile == "-" || tile == "|" || tile == "#" || tile == "@" || tile == "M") { log_msg = "Path blocked."; continue; }
    
    if (tile == "*") { gems++; log_msg = "Got Gem!"; }
    else if (tile == "V") { gems += 3; log_msg = "Got Vein!"; }
    else if (tile == "%") { p_hp = min(max_hp, p_hp + 10); log_msg = "Healed 10 HP!"; }
    else if (tile == "$") { int bonus = rand() % 4 + 2; gems += bonus; log_msg = "Chest open! +" + to_string(bonus) + " Gems."; }
    else if (tile == "&") { has_amulet = true; log_msg = "SECURED AMULET! Run!"; }
    else log_msg = "Moved.";
    
    if (ny == 2 && nx == 2 && has_amulet) {
      system("clear");
      cout << "=============" << endl;
      cout << "you escaped " << "the first mine" << endl;
cout << "=============" << endl;cout << " Gems: " << gems << endl;cout << " Slain: " << monsters_slain << endl;cout << " SCORE: " << calculate_score(true) << endl;cout << "=============" << endl;break;}if ((p_y == 5 || p_y == 14) && (p_x == 10 || p_x == 30)) mine_map[p_y][p_x] = "+";else if ((p_x == 15 || p_x == 25) && (p_y == 3 || p_y == 10 || p_y == 17)) mine_map[p_y][p_x] = "+";else mine_map[p_y][p_x] = ".";p_x = nx; p_y = ny; mine_map[p_y][p_x] = "Y";// TRIGGER MONSTER TURN AFTER PLAYER MOVESmonster_turn();}return 0;}

        } 
} 
