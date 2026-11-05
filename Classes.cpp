
#include "Classes.h"
#include <cctype>
#include <cstdlib>
#ifdef _WIN32
#include <conio.h>     
#endif

namespace Delivery {

    vector<string> split(const string& line) {
        vector<string> fields;
        istringstream instr(line);    // строковый поток 
        string field;
        while (getline(instr, field, ';'))
            fields.push_back(field);
        return fields;
    }

    string toLower(string text) {
        for (size_t i = 0; i < text.size(); i++)
            text[i] = tolower((unsigned char)text[i]);
        return text;
    }

    bool isValidPhone(const string& phone) {
        if (phone.size() != 13 || phone.substr(0, 4) != "+375")
            return false;
        for (size_t i = 4; i < phone.size(); i++)
            if (!isdigit((unsigned char)phone[i]))
                return false;
        return true;
    }

    string inputString(const string& prompt) {
        string text;
        while (true) {
            cout << prompt;
            getline(cin, text);
            if (cin.eof()) exit(0);
            if (text.empty())
                cout << "Ошибка! Строка не может быть пустой.\n";
            else if (text.find(';') != string::npos)
                cout << "Ошибка! Нельзя использовать символ ';'.\n";
            else
                return text;
        }
    }

    // ывод пароля
    string inputPassword(const string& prompt) {
        cout << prompt;
        string password;
#ifdef _WIN32
        int ch;
        while ((ch = _getch()) != '\r') {       
            if (ch == 0 || ch == 224) {        
                _getch();
                continue;
            }
            if (ch == '\b') {                   
                if (!password.empty()) {
                    password.erase(password.size() - 1);
                    cout << "\b \b";
                }
            }
            else {
                password += (char)ch;
                cout << '*';
            }
        }
#else
        //  отключаем отображение символов и построчный ввод в терминале
        system("stty -echo -icanon 2>/dev/null");
        char ch;
        while (cin.get(ch) && ch != '\n') {
            if (ch == 127 || ch == '\b') {
                if (!password.empty()) {
                    password.erase(password.size() - 1);
                    cout << "\b \b" << flush;
                }
            }
            else {
                password += ch;
                cout << '*' << flush;
            }
        }
        system("stty echo icanon 2>/dev/null");
        if (cin.eof()) exit(0);
#endif
        cout << endl;
        return password;
    }



    int Entity::nextId = 1;

    Entity::Entity() {
        id = nextId++;
    }

    Entity::Entity(int id) {
        setId(id);
    }

    void Entity::setId(int newId) {
        id = newId;
        if (newId >= nextId)       // следующий новый номер будет больше всех прочитанных
            nextId = newId + 1;
    }


    User::User(const string& login, const string& password, const string& name, const string& phone)
        : login(login), name(name), phone(phone) {
        if (login.size() < 3)
            throw InputException("логин должен содержать не менее 3 символов");
        if (password.size() < 4)
            throw InputException("пароль должен содержать не менее 4 символов");
        if (!isValidPhone(phone))
            throw InputException("телефон должен быть в формате +375XXXXXXXXX");
        passwordHash = hashPassword(password);
    }

    // Строка файла: роль;id;логин;хеш;ФИО;телефон;итд
    User::User(const vector<string>& f)
        : Entity(stoi(f[1])), login(f[2]), passwordHash(f[3]), name(f[4]), phone(f[5]) {
    }

    // хеширование (алгоритм djb2):
    string User::hashPassword(const string& password) {
        unsigned long long hash = 5381;
        for (size_t i = 0; i < password.size(); i++)
            hash = hash * 33 + (unsigned char)password[i];
        return to_string(hash);
    }

    bool User::checkPassword(const string& password) const {
        return hashPassword(password) == passwordHash;
    }

    // по роли из файла создаёт объект нужного класса
    shared_ptr<User> User::fromLine(const string& line) {
        vector<string> f = split(line);
        if (f.size() == 7 && f[0] == "Клиент")
            return make_shared<Client>(f);
        if (f.size() == 8 && f[0] == "Курьер")
            return make_shared<Courier>(f);
        if (f.size() == 6 && f[0] == "Администратор")
            return make_shared<Administrator>(f);
        throw FileException("неверная строка пользователя");
    }

    void User::print() const {
        cout << setw(5) << id << setw(12) << login << setw(28) << name
            << setw(15) << phone << setw(15) << getRole();
    }

    string User::toLine() const {
        return getRole() + ";" + to_string(id) + ";" + login + ";" + passwordHash + ";"
            + name + ";" + phone;
    }

    Client::Client(const string& login, const string& password, const string& name,
        const string& phone, const string& address)
        : User(login, password, name, phone), address(address) {
    }

    Client::Client(const vector<string>& f) : User(f), address(f[6]) {}

    void Client::print() const {
        User::print();
        cout << address;
    }

    string Client::toLine() const {
        return User::toLine() + ";" + address;
    }


    Courier::Courier(const string& login, const string& password, const string& name, const string& phone)
        : User(login, password, name, phone), busy(false), deliveredCount(0) {
    }

    Courier::Courier(const vector<string>& f)
        : User(f), busy(f[6] == "1"), deliveredCount(stoi(f[7])) {
    }

    void Courier::print() const {
        User::print();
        cout << (busy ? "занят" : "свободен") << ", доставок: " << deliveredCount;
    }

    string Courier::toLine() const {
        return User::toLine() + ";" + (busy ? "1" : "0") + ";" + to_string(deliveredCount);
    }

 



    Administrator::Administrator(const string& login, const string& password,
        const string& name, const string& phone)
        : User(login, password, name, phone) {
    }

    Administrator::Administrator(const vector<string>& f) : User(f) {}

    



    Restaurant::Restaurant(const string& name, const string& address, const string& phone)
        : name(name), address(address) {
        setPhone(phone);
    }

    void Restaurant::setPhone(const string& value) {
        if (!isValidPhone(value))
            throw InputException("телефон должен быть в формате +375XXXXXXXXX");
        phone = value;
    }

    void Restaurant::print() const {
        cout << *this;    

    }




    string Restaurant::toLine() const {
        return to_string(id) + ";" + name + ";" + address + ";" + phone;
    }


    shared_ptr<Restaurant> Restaurant::fromLine(const string& line) {
        shared_ptr<Restaurant> restaurant = make_shared<Restaurant>();
        istringstream instr(line);
        instr >> *restaurant;           
        if (instr.fail())               
            throw FileException("неверная строка ресторана");
        return restaurant;
    }

    ostream& operator<<(ostream& os, const Restaurant& r) {
        os << setw(5) << r.id << setw(22) << r.name << setw(15) << r.phone << r.address;
        return os;
    }

    
    istream& operator>>(istream& is, Restaurant& r) {
        string line;
        getline(is, line);
        vector<string> f = split(line);
        if (f.size() != 4) {
            is.setstate(ios::failbit); 
            return is;
        }
        try {
            r.setId(stoi(f[0]));
        }
        catch (const exception&) {
            is.setstate(ios::failbit);
            return is;
        }
        r.name = f[1];
        r.address = f[2];
        r.phone = f[3];
        return is;
    }

   




    Dish::Dish(const string& name, const string& category, double price, int restaurantId)
        : name(name), category(category), restaurantId(restaurantId) {
        setPrice(price);
    }


    void Dish::setPrice(double value) {
        if (value <= 0)
            throw InputException("цена должна быть больше нуля");
        price = value;
    }

    void Dish::print() const {
        cout << *this;    
    }

    string Dish::toLine() const {
        ostringstream outstr;
        outstr << id << ";" << name << ";" << category << ";"
            << fixed << setprecision(2) << price << ";" << restaurantId;
        return outstr.str();
    }

    shared_ptr<Dish> Dish::fromLine(const string& line) {
        shared_ptr<Dish> dish = make_shared<Dish>();
        istringstream instr(line);
        instr >> *dish;                 
        if (instr.fail())              
            throw FileException("неверная строка блюда");
        return dish;
    }

    ostream& operator<<(ostream& os, const Dish& dish) {
        os << setw(5) << dish.id << setw(24) << dish.name << setw(14) << dish.category
            << setw(10) << dish.price;
        return os;
    }

    // чтение блюда из строки файла: id;название;категория;цена;id ресторана итд
    istream& operator>>(istream& is, Dish& dish) {
        string line;
        getline(is, line);
        vector<string> f = split(line);
        if (f.size() != 5) {
            is.setstate(ios::failbit);
            return is;
        }
        try {
            dish.setId(stoi(f[0]));
            dish.price = stod(f[3]);
            dish.restaurantId = stoi(f[4]);
        }
        catch (const exception&) {
            is.setstate(ios::failbit);
            return is;
        }
        dish.name = f[1];
        dish.category = f[2];
        return is;
    }

    bool operator<(const Dish& d1, const Dish& d2) {
        return toLower(d1.name) < toLower(d2.name);
    }

   



    const double Order::DELIVERY_PRICE = 5.0;

    Order::Order(int clientId, int restaurantId, const string& address)
        : clientId(clientId), courierId(0), restaurantId(restaurantId),
        status(WAITING), address(address) {
    }

    // строка файла: id;клиент;курьер;ресторан;статус;адрес;кол-во позиций;название;цена;кол-во;пупупу
    Order::Order(const vector<string>& f)
        : Entity(stoi(f[0])), clientId(stoi(f[1])), courierId(stoi(f[2])),
        restaurantId(stoi(f[3])), status(stoi(f[4])), address(f[5]) {
        int count = stoi(f[6]);
        if ((int)f.size() != 7 + count * 3)
            throw FileException("неверное число позиций заказа");
        for (int i = 0; i < count; i++) {
            OrderItem item;
            item.dishName = f[7 + i * 3];
            item.price = stod(f[8 + i * 3]);
            item.quantity = stoi(f[9 + i * 3]);
            items.push_back(item);
        }
    }

    void Order::addItem(const OrderItem& item) {
        // если такое блюдо уже есть в заказе - увеличиваем количество
        for (size_t i = 0; i < items.size(); i++) {
            if (items[i].dishName == item.dishName) {
                items[i].quantity += item.quantity;
                return;
            }
        }
        items.push_back(item);
    }

    void Order::addItem(const Dish& dish, int quantity) {
        if (dish.getRestaurantId() != restaurantId)
            throw InputException("блюдо из другого ресторана");
        OrderItem item;
        item.dishName = dish.getName();
        item.price = dish.getPrice();
        item.quantity = quantity;
        addItem(item);
    }

    double Order::getSum() const {
        double sum = 0;
        for (size_t i = 0; i < items.size(); i++)
            sum += items[i].price * items[i].quantity;
        return sum;
    }

    double Order::getTotal() const {
        return getSum() + DELIVERY_PRICE;
    }

    void Order::setStatus(int newStatus) {
        bool allowed = (status == WAITING && (newStatus == DELIVERING || newStatus == CANCELLED))
            || (status == DELIVERING && newStatus == DELIVERED);
        if (!allowed)
            throw DeliveryException("Нельзя сменить статус \"" + statusToString(status)
                + "\" на \"" + statusToString(newStatus) + "\"");
        status = newStatus;
    }

    void Order::printCheck(ostream& os, const string& restaurantName) const {
        os << "==================== ЧЕК ЗАКАЗА № " << id << " ====================\n";
        os << "Ресторан: " << restaurantName << "\n";
        os << "Адрес доставки: " << address << "\n";
        os << "Статус: " << statusToString(status) << "\n";
        os << left << setw(26) << "Блюдо" << setw(10) << "Цена" << setw(8) << "Кол-во" << "Сумма\n";
        for (size_t i = 0; i < items.size(); i++) {
            os << setw(26) << items[i].dishName << setw(10) << items[i].price
                << setw(8) << items[i].quantity << items[i].price * items[i].quantity << "\n";
        }
        os << "Сумма заказа: " << getSum() << "\n";
        os << "Доставка:     " << DELIVERY_PRICE << "\n";
        os << "ИТОГО:        " << getTotal() << " BYN\n";
        os << "============================================================\n";
    }

    void Order::print() const {
        cout << setw(6) << id << setw(12) << statusToString(status)
            << setw(10) << getTotal() << address;
    }

    string Order::toLine() const {
        ostringstream outstr;
        outstr << id << ";" << clientId << ";" << courierId << ";" << restaurantId << ";"
            << status << ";" << address << ";" << items.size();
        for (size_t i = 0; i < items.size(); i++)
            outstr << ";" << items[i].dishName << ";" << fixed << setprecision(2)
            << items[i].price << ";" << items[i].quantity;
        return outstr.str();
    }

    shared_ptr<Order> Order::fromLine(const string& line) {
        vector<string> f = split(line);
        if (f.size() < 7)
            throw FileException("неверная строка заказа");
        return make_shared<Order>(f);
    }

   

    RevenueReport::RevenueReport(int size) : size(size) {
        names = new string[size];   
        revenue = new double[size];
        count = new int[size];
        for (int i = 0; i < size; i++) {
            revenue[i] = 0;
            count[i] = 0;
        }
    }

    RevenueReport::~RevenueReport() {
        delete[] names;              
        delete[] revenue;
        delete[] count;
    }

    void RevenueReport::setName(int index, const string& name) {
        names[index] = name;
    }

    void RevenueReport::addOrder(int index, double sum) {
        revenue[index] += sum;
        count[index]++;
    }

    double RevenueReport::getTotal() const {
        double total = 0;
        for (int i = 0; i < size; i++)
            total += revenue[i];
        return total;
    }

    ostream& operator<<(ostream& os, const RevenueReport& report) {
        os << left << fixed << setprecision(2);
        os << "ОТЧЁТ О ВЫРУЧКЕ РЕСТОРАНОВ (доставленные заказы)\n";
        os << setw(22) << "Ресторан" << setw(10) << "Заказов" << "Выручка, BYN\n";
        for (int i = 0; i < report.size; i++)
            os << setw(22) << report.names[i] << setw(10) << report.count[i] << report.revenue[i] << "\n";
        os << "Итого: " << report.getTotal() << " BYN\n";
        return os;
    }

}