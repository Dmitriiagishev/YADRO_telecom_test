#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <tuple>
#include <algorithm>

std::ofstream file_out;
std::ostream* out = &*out;  

// Проверка на наличие неразрешенных символов
bool check_chars(std::string in_str){
    std::string allowed = " 0123456789,irongldemsxp";
    for (int i = 0; i < in_str.length(); i++){
        if (allowed.find(in_str[i]) == -1) return false;
    }
    return true;
}
// Деление входной строки на компоненты
std::vector<std::string> split(std::string in_str, char delimiter)
{
    
    std::stringstream ss(in_str);
    std::vector<std::string> res;
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        res.push_back(token);
    }
    return res;
}
// Деление строки на соседей
std::vector<int> split(std::string in_str)
{
    std::stringstream ss(in_str);
    std::vector<int> res;
    std::string token;
    while (std::getline(ss, token, ',')) {
        res.push_back(std::stoi(token));
    }
    return res;
}
// Класс команты в подземелье
class Room{
public:
    std::unordered_map<std::string, int> resources;
    std::vector<int> neighbours;
    bool first_time = true;
    // Констуктор по умолчанию
    Room(){
        resources["iron"] = 0;
        resources["gold"] = 0;
        resources["gems"] = 0;
        resources["exp"] = 0;
    }
    // Конструктор
    Room(std::vector<int> neighbours, int iron, int gold, int gems, int exp){
        this->neighbours = neighbours;
        resources["iron"] = iron;
        resources["gold"] = gold;
        resources["gems"] = gems;
        resources["exp"] = exp;
    }
    // Проверка на наличие ресурсов в комнате
    bool has_resources(){
        return resources["iron"] + resources["gold"] + resources["gems"] + resources["exp"];
    }
};

std::ostream& operator<<(std::ostream& os, const Room& room) {
    std::vector<std::string> keys = {"iron", "gold", "gems", "exp"};
    
    for (size_t i = 0; i < keys.size(); ++i) {
        if (i > 0) {
            os << " ";
        }
        
        int value = room.resources.at(keys[i]);
        if (value == -1) {
            os << "_";
        } else {
            os << value;
        }
    }
    
    return os;
}
// Класс раскладки в подземельх
class Rooms{
public:
    std::unordered_map<int, Room> rooms;
    
    Rooms(){
    }
    // Добавление комнаты в подземелье
    bool add_room(std::string in_str){
        std::vector<std::string> in_numbers = split(in_str, ' ');
        if (in_numbers.size() != 6) return false;
        int exp = std::stoi(in_numbers[5]);
        int gems = std::stoi(in_numbers[4]);
        int gold = std::stoi(in_numbers[3]);
        int iron = std::stoi(in_numbers[2]);
        std::vector<int> neighbours = split(in_numbers[1]);
        int room_number = std::stoi(in_numbers[0]);

        rooms[room_number] = Room(neighbours, iron, gold, gems, exp);
        return true;
    }
    // Поиск кратчайшего пути между комнатами чере обход в ширину
    std::vector<int> find_path(int start, int finish){
        std::unordered_map<int, int> parents;
        std::vector<int> queue;
        
        
        queue.push_back(start);
        parents[start] = -1;  
        
        size_t c = 0;
        bool found = false;
        
        while (c < queue.size() && !found) {
            int current_room = queue[c];
            
            for (size_t i = 0; i < rooms[current_room].neighbours.size(); i++) {
                int neighbour = rooms[current_room].neighbours[i];
                
                if (parents.find(neighbour) == parents.end()&& 
                    (!rooms[neighbour].first_time || neighbour == 0)) {
                    parents[neighbour] = current_room;
                    queue.push_back(neighbour);
                    
                    if (neighbour == finish) {
                        found = true;
                        break;
                    }
                }
            }
            c++;
        }
        
        if (parents.find(finish) == parents.end()) {
            return std::vector<int>();
        }
        
        std::vector<int> path;
        int current = finish;
        
        while (current != start) {
            path.push_back(current);
            current = parents[current];
        }
        
        
        
        std::reverse(path.begin(), path.end());
        
        return path;
    }
    // Поиск кратчайшего пути до начальной комнаты через пройденные
    std::vector<int> find_way_back(int start){
        std::unordered_map<int, int> parents;
        std::vector<int> queue;
        
        queue.push_back(0);
        parents[0] = -1;
        
        size_t c = 0;
        bool found = false;
        
        while (c < queue.size() && !found) {
            int current_room = queue[c];
            
            for (size_t i = 0; i < rooms[current_room].neighbours.size(); i++) {
                int neighbour = rooms[current_room].neighbours[i];
                
                if (parents.find(neighbour) == parents.end() && 
                    (!rooms[neighbour].first_time || neighbour == 0)) {
                    parents[neighbour] = current_room;
                    queue.push_back(neighbour);
                    
                    if (neighbour == start) {
                        found = true;
                        break;
                    }
                }
            }
            c++;
        }
        
        if (parents.find(start) == parents.end()) {
            return std::vector<int>();
        }
        
        std::vector<int> path;
        int current = start;
        
        while (current != -1) {  
            path.push_back(current);
            current = parents[current];
        }
        
        return path;
    }
};
// Класс бота для исследования
class Bot{
public:
    Rooms rooms;
    int rooms_amount = 0;
    int current_room = 0;
    int start_food = 0;
    int current_food = 0;
    int collected_resources = 0;
    std::string priority = "";
    std::unordered_map<std::string, int> collected_deposits = {{"iron", 0}, {"gold", 0}, {"gems", 0}, {"exp", 0}};
    std::unordered_map<std::string, int> values = {{"iron", 7}, {"gold", 11}, {"gems", 23}, {"exp", 1}};
    std::unordered_map<int, bool> unvisited_rooms;

    Bot(){
    }

    int read_file(std::string input_file){
        try{
            std::ifstream in_file(input_file);
            if (!in_file.is_open()) {
                std::cerr << "Error opening the file!";
                return 1;
            }
            std::string s;

            getline(in_file, s);
            if (!check_chars(s)){
                *out << s << '\n';
                return 1;
            }
            rooms_amount = std::stoi(s);

            for (int i = 0; i <= rooms_amount; i++)  {
                getline(in_file, s);
                if (!check_chars(s)){
                    *out << s << '\n';
                    return 1;
                }
                if (rooms.add_room(s)){
                    std::vector<std::string> parts = split(s, ' ');
                    unvisited_rooms[std::stoi(parts[0])] = true;
                } else {
                    *out << s << '\n';
                    return 1;
                }
            }
            unvisited_rooms[0] = false;
            
            getline(in_file, s);
            if (!check_chars(s)){
                *out << s << '\n';
                return 1;
            }
            std::vector<std::string> start_info = split(s, ' ');
            start_food = std::stoi(start_info[0]);
            current_food = start_food;
            priority = start_info[1];
            values[priority] *= 2;
            
            
            in_file.close();
        }
        catch (...) {
            return 1;
        }
        return 0;
    }
    // Переход в комнату (по алгоритму, в ближайшую)
    bool switch_room(){
        std::vector<int> available_rooms = rooms.rooms[current_room].neighbours;
        for (size_t i = 0; i < available_rooms.size(); i++){
            if (unvisited_rooms[available_rooms[i]]){
                unvisited_rooms[available_rooms[i]] = false;
                current_room = available_rooms[i];
                current_food--;
                *out << "go " << current_room << '\n';
                return true;
            }
        }
        
        std::vector<int> path;
        for (int i = 0; i < this->rooms_amount; i++){
            if (this->unvisited_rooms[i]) {
                path = this->rooms.find_path(this->current_room, i);
                break;
            }
        }
         
        for (size_t i = 0; i < path.size(); i ++){
            this->current_room = path[i];
            this->current_food--;
        }
        return false;
    }
    // Переход в соседнюю комнату с конкретным номером
    bool switch_room(int next_room){
        unvisited_rooms[next_room] = false;
        current_room = next_room;
        current_food--;
        *out << "go " << current_room << '\n';
        return true;
    }
    // Сбор лучших оставшихся ресурсов в комнате
    void collect_resources(){
        std::vector<std::string> resources = {"iron", "gold", "gems", "exp"};
        int max_value = 0;
        int needed = 0;
        for (int i = 0; i < 4; i ++){
            int deposit = this->values[resources[i]] * this->rooms.rooms[this->current_room].resources[resources[i]];
            
            if (max_value < deposit){
                max_value = deposit;
                needed = i;
            }
            
        } 
        *out << "collect " << resources[needed] <<'\n';
        this->collected_resources += max_value;
        this->collected_deposits[resources[needed]] += this->rooms.rooms[this->current_room].resources[resources[needed]];
        this->rooms.rooms[this->current_room].resources[resources[needed]] = -1;
        if (rooms.rooms[this->current_room].first_time){
            rooms.rooms[this->current_room].first_time = false;
        } else current_food --;
    }
    // Вывод состояния
    void get_state(){
        if (current_room) *out << "state " << current_room << ' ' << rooms.rooms[current_room] << '\n';
    }
    // Основной алгоритм работы
    void explore_dungeon(){
        // Изначальное исследование подземелья
        while (current_food > start_food / 2 + (start_food % 2)){
            switch_room();
            get_state();
            collect_resources();
            get_state();
        }

        std::vector<int> path = rooms.find_way_back(current_room);  
        int spare_food = current_food - path.size() + 1;
        // Возвращение
        for (int i = 1; i < path.size(); i++){
            
            while (spare_food > 0 && rooms.rooms[current_room].has_resources()){
                collect_resources();
                get_state();
                spare_food --;
            }
            switch_room(path[i]);
            get_state();
        }
        *out << "result " << collected_deposits["iron"] << " " << collected_deposits["gold"] << " " << collected_deposits["gems"] << " " << collected_deposits["exp"] << " " << collected_resources << '\n';
    }
    
};

int main(int argc, char *argv[]){
    // Для запуска
    Bot bot;
    
    file_out.open("result.txt");
    if (file_out.is_open()) {
            out = &file_out;  // Можно закомментировать и будет вывод в консоль
        }
    
    if (argc < 2){
        std::cerr << "No input file specified\n";
        return 1;
    }
    if (bot.read_file(argv[1])){
        return 1;
    }

   
    bot.explore_dungeon();

    
    return 0;
}