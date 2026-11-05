#pragma once
// Classes.h - классы предметной области "Служба доставки еды"
#pragma once
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
using namespace std;

namespace Delivery {  // собственное пространство имён

    // Базовый класс исключений программы
    class DeliveryException : public exception {
    protected:
        string message;
    public:
        DeliveryException(const string& msg) : message(msg) {}
        const char* what() const noexcept override { return message.c_str(); }
    };

    // Ошибка введённых данных
    class InputException : public DeliveryException {
    public:
        InputException(const string& msg) : DeliveryException("Ошибка ввода: " + msg) {}
    };

    // Ошибка работы с файлом
    class FileException : public DeliveryException {
    public:
        FileException(const string& msg) : DeliveryException("Ошибка файла: " + msg) {}
    };

    // Запись не найдена
    class NotFoundException : public DeliveryException {
    public:
        NotFoundException(const string& msg) : DeliveryException("Не найдено: " + msg) {}
    };

    //==================== Вспомогательные функции ====================

    // Встроенная функция: пропуск остатка строки ввода (из листинга темы 4)
    inline void eatline() { while (cin.get() != '\n') continue; }

    vector<string> split(const string& line);  // разбивает строку файла на поля по ';'
    string toLower(string text);               // перевод в нижний регистр (параметр по значению)
    bool isValidPhone(const string& phone);    // проверка формата +375XXXXXXXXX
    string inputString(const string& prompt);  // ввод непустой строки без символа ';'
    string inputPassword(const string& prompt); // ввод пароля со звёздочками

    // Шаблон функции: ввод числа любого типа в диапазоне [min; max]
    template <class T>
    T inputNumber(const string& prompt, T min, T max) {
        T value;
        while (true) {
            cout << prompt;
            cin >> value;
            if (cin.eof()) exit(0);          // ввод закрыт
            if (!cin) {                      // введено не число
                cin.clear();
                eatline();
                cout << "Ошибка! Введите число.\n";
            }
            else if (value < min || value > max) {
                eatline();
                cout << "Ошибка! Введите число от " << min << " до " << max << ".\n";
            }
            else {
                eatline();
                return value;
            }
        }
    }

    // Статусы заказа
    enum OrderStatus { WAITING, DELIVERING, DELIVERED, CANCELLED };

    inline string statusToString(int status) {
        switch (status) {
        case WAITING:    
            return "Ожидает";
        case DELIVERING: 
            return "В пути";
        case DELIVERED:  
            return "Доставлен";
        case CANCELLED:  
            return "Отменён";
        }
        return "?";
    }

    //==================== Базовый класс ====================

    // Абстрактный класс "Запись" (уровень 1)
    class Entity {
    protected:
        int id;
        static int nextId;     // статическое поле: следующий свободный номер
        void setId(int newId); // номер из файла
    public:
        Entity();              // новая запись - получает следующий номер
        Entity(int id);        // запись из файла - номер уже известен
        virtual ~Entity() {}
        int getId() const {
            return id;
        }
        virtual void print() const = 0;        // вывод на экран
        virtual string toLine() const = 0;     // строка для записи в файл
    };

    //==================== Пользователи ====================

    // Абстрактный класс "Пользователь" (уровень 2)
    class User : public Entity {
    protected:
        string login;
        string passwordHash;   // хранится только хеш пароля
        string name;
        string phone;
    public:
        User(const string& login, const string& password, const string& name, const string& phone);
        User(const vector<string>& fields);    // создание из строки файла
        virtual string getRole() const = 0;    // чисто виртуальная функция

        string getName() const { 
            return login;
        }
        string getFullName() const { 
            return name; 
        }
        string getPhone() const {
            return phone; 
        }
        bool checkPassword(const string& password) const;

        static string hashPassword(const string& password);    // статический метод
        static shared_ptr<User> fromLine(const string& line);  // фабричный метод

        void print() const override;
        string toLine() const override;
    };

    // Клиент (уровень 3)
    class Client : public User {
        string address;
    public:
        Client(const string& login, const string& password, const string& name,
            const string& phone, const string& address);
        Client(const vector<string>& fields);
        string getRole() const override {
            return "Клиент"; 
        }
        string getAddress() const { 
            return address;
        }
        void print() const override;
        string toLine() const override;
    };

    // Курьер (уровень 3)
    class Courier : public User {
        bool busy;           // занят доставкой
        int deliveredCount;  // выполнено доставок
    public:
        Courier(const string& login, const string& password, const string& name, const string& phone);
        Courier(const vector<string>& fields);
        string getRole() const override { 
            return "Курьер";
        }
        bool isBusy() const { 
            return busy;
        }
        void setBusy(bool value) { 
            busy = value;
        }
        void addDelivery() { 
            deliveredCount++;
        }
        void print() const override;
        string toLine() const override;
    };

    // Администратор (уровень 3)
    class Administrator : public User {
    public:
        Administrator(const string& login, const string& password, const string& name, const string& phone);
        Administrator(const vector<string>& fields);
        string getRole() const override { 
            return "Администратор"; 
        
        }
    };

    //==================== Рестораны и блюда ====================

    class Restaurant : public Entity {
        string name;
        string address;
        string phone;
    public:
        Restaurant() : Entity(0) {}  // пустой объект для чтения из файла
        Restaurant(const string& name, const string& address, const string& phone);
        string getName() const { 
            return name; 
        }
        void setName(const string& value) {
            name = value; 
        }
        void setAddress(const string& value) { 
            address = value; 
        }
        void setPhone(const string& value);
        void print() const override;
        string toLine() const override;
        static shared_ptr<Restaurant> fromLine(const string& line);

        // Перегрузка операторов ввода/вывода
        friend ostream& operator<<(ostream& os, const Restaurant& restaurant);
        friend istream& operator>>(istream& is, Restaurant& restaurant);
    };

    class Dish : public Entity {
        string name;
        string category;
        double price;
        int restaurantId;
    public:
        Dish() : Entity(0), price(0), restaurantId(0) 
        {}  // пустой объект для чтения из файла
        Dish(const string& name, const string& category, double price, int restaurantId);
        string getName() const {
            return name; 
        }
        double getPrice() const { 
            return price;
        }
        int getRestaurantId() const {
            return restaurantId;
        }
        void setName(string value) { 
            name = value; 
        }   // передача по значению
        void setCategory(const string& value) { 
            category = value;
        }
        void setPrice(double value);
        void print() const override;
        string toLine() const override;
        static shared_ptr<Dish> fromLine(const string& line);

        // Дружественные функции: перегрузка операторов
        friend ostream& operator<<(ostream& os, const Dish& dish);
        friend istream& operator>>(istream& is, Dish& dish);
        friend bool operator<(const Dish& d1, const Dish& d2);   // сравнение по названию
    };

    //==================== Заказ ====================

    // Позиция заказа: название и цена запоминаются на момент заказа
    struct OrderItem {
        string dishName;
        double price;
        int quantity;
    };

    class Order : public Entity {
        int clientId;
        int courierId;       // 0 - курьер не назначен
        int restaurantId;
        int status;
        string address;
        vector<OrderItem> items;
    public:
        static const double DELIVERY_PRICE;    // стоимость доставки

        Order(int clientId, int restaurantId, const string& address);
        Order(const vector<string>& fields);

        // Перегрузка методов
        void addItem(const OrderItem& item);
        void addItem(const Dish& dish, int quantity);

        double getSum() const;     // сумма блюд
        double getTotal() const;   // сумма с доставкой
        void setStatus(int newStatus);
        void setCourier(int id) {
            courierId = id;
        }

        int getClientId() const {
            return clientId;
        }
        int getCourierId() const { 
            return courierId;
        }
        int getRestaurantId() const { 
            return restaurantId;
        }
        int getStatus() const { 
            return status; 
        }
        string getAddress() const {
            return address; 
        }
        bool isEmpty() const { 
            return items.empty();
        }

        void printCheck(ostream& os, const string& restaurantName) const;  // чек в консоль или в файл
        void print() const override;
        string toLine() const override;
        static shared_ptr<Order> fromLine(const string& line);
    };

    //==================== Отчёт о выручке ====================

    // Массивы выделяются динамически в конструкторе и освобождаются в деструкторе
    class RevenueReport {
        string* names;     // названия ресторанов
        double* revenue;   // выручка каждого ресторана
        int* count;        // количество доставленных заказов
        int size;
    public:
        RevenueReport(int size);
        ~RevenueReport();
        RevenueReport(const RevenueReport&) = delete;             // копирование запрещено,
        RevenueReport& operator=(const RevenueReport&) = delete;  // чтобы память не удалилась дважды
        void setName(int index, const string& name);
        void addOrder(int index, double sum);
        double getTotal() const;
        friend ostream& operator<<(ostream& os, const RevenueReport& report);
    };

}
