
#include "DeliverySystem.h"
#include <queue>

namespace Delivery {


    bool cheaperThan(const shared_ptr<Dish>& d1, const shared_ptr<Dish>& d2) {
        return d1->getPrice() < d2->getPrice();
    }

    bool byName(const shared_ptr<Dish>& d1, const shared_ptr<Dish>& d2) {
        return *d1 < *d2;    
    }


    class FromRestaurant {
        int restaurantId;
    public:
        FromRestaurant(int id) : restaurantId(id) {}
        bool operator()(const shared_ptr<Dish>& dish) const {
            return dish->getRestaurantId() == restaurantId;
        }
    };

    class PriceRange {
        double min, max;
    public:
        PriceRange(double min, double max) : min(min), max(max) {}
        bool operator()(const shared_ptr<Dish>& dish) const {
            return dish->getPrice() >= min && dish->getPrice() <= max;
        }
    };


    class WithStatus {
        int status;
    public:
        WithStatus(int status) : status(status) {}
        bool operator()(const shared_ptr<Order>& order) const {
            return order->getStatus() == status;
        }
    };

   

    DeliverySystem::DeliverySystem()
        : users("users.txt"), restaurants("restaurants.txt"),
        dishes("dishes.txt"), orders("orders.txt") {
    }

    DeliverySystem& DeliverySystem::getInstance() {
        static DeliverySystem system;    
        return system;
    }

    void DeliverySystem::loadAll() {
        users.load();
        restaurants.load();
        dishes.load();
        orders.load();
    }

    void DeliverySystem::saveAll() {
        users.save();
        restaurants.save();
        dishes.save();
        orders.save();
    }

   
    void DeliverySystem::createDemoData() {
        users.add(make_shared<Administrator>("admin", "admin123", "Иванов Иван Иванович", "+375291111111"));
        users.add(make_shared<Courier>("courier", "courier123", "Петров Пётр Петрович", "+375292222222"));
        users.add(make_shared<Client>("client", "client123", "Сидорова Анна Сергеевна", "+375293333333",
            "г. Минск, ул. Независимости, д. 4, кв. 15"));

        shared_ptr<Restaurant> pizza = make_shared<Restaurant>("Пицца Темпо", "г. Минск, ул. Немига, 12", "+375175551010");
        shared_ptr<Restaurant> sushi = make_shared<Restaurant>("Суши Мастер", "г. Минск, ул. Притыцкого, 29", "+375175552020");
        restaurants.add(pizza);
        restaurants.add(sushi);

        dishes.add(make_shared<Dish>("Маргарита", "Пицца", 18.90, pizza->getId()));
        dishes.add(make_shared<Dish>("Пепперони", "Пицца", 22.50, pizza->getId()));
        dishes.add(make_shared<Dish>("Паста Карбонара", "Паста", 16.40, pizza->getId()));
        dishes.add(make_shared<Dish>("Лимонад", "Напитки", 4.50, pizza->getId()));
        dishes.add(make_shared<Dish>("Филадельфия", "Роллы", 24.00, sushi->getId()));
        dishes.add(make_shared<Dish>("Калифорния", "Роллы", 19.50, sushi->getId()));
        dishes.add(make_shared<Dish>("Мисо-суп", "Супы", 7.80, sushi->getId()));
        dishes.add(make_shared<Dish>("Зелёный чай", "Напитки", 3.90, sushi->getId()));
    }

  

    void DeliverySystem::run() {
        cout << left << fixed << setprecision(2);   // формат вывода таблиц и цен
        try {
            loadAll();
            if (users.size() == 0) {     // первый запуск
                createDemoData();
                saveAll();
            }
        }
        catch (const DeliveryException& e) {
            cout << e.what() << endl;
            cout << "Исправьте или удалите повреждённый файл и запустите программу снова.\n";
            return;
        }

        while (true) {
            cout << "\n========== СЛУЖБА ДОСТАВКИ ЕДЫ ==========\n";
            cout << "1. Вход\n";
            cout << "2. Регистрация клиента\n";
            cout << "0. Выход из программы\n";
            int choice = inputNumber("Ваш выбор: ", 0, 2);
            if (choice == 0)
                break;
            try {
                if (choice == 1)
                    login();
                else
                    registerClient();
            }
            catch (const DeliveryException& e) {
                cout << e.what() << endl;
            }
        }
        cout << "До свидания!\n";
    }

   

    bool DeliverySystem::loginExists(const string& login) {
        for (int i = 0; i < users.size(); i++)
            if (users.getAll()[i]->getName() == login)
                return true;
        return false;
    }

    void DeliverySystem::login() {
        string login = inputString("Логин: ");
        string password = inputPassword("Пароль: ");
        currentUser = nullptr;
        for (int i = 0; i < users.size(); i++) {
            shared_ptr<User> user = users.getAll()[i];
            if (user->getName() == login && user->checkPassword(password))
                currentUser = user;
        }
        if (currentUser == nullptr) {
            cout << "Неверный логин или пароль.\n";
            return;
        }
        cout << "Добро пожаловать, " << currentUser->getFullName()
            << " (" << currentUser->getRole() << ")!\n";

        // меню выбирается по роли пользователя
        if (currentUser->getRole() == "Клиент")
            clientMenu();
        else if (currentUser->getRole() == "Курьер")
            courierMenu();
        else
            adminMenu();
        currentUser = nullptr;
    }

    void DeliverySystem::registerClient() {
        cout << "\n--- Регистрация клиента ---\n";
        string login = inputString("Логин (не менее 3 символов): ");
        if (login.size() < 3)
            throw InputException("логин должен содержать не менее 3 символов");
        if (loginExists(login))
            throw InputException("логин \"" + login + "\" уже занят");
        string password = inputPassword("Пароль (не менее 4 символов): ");
        if (password.size() < 4)
            throw InputException("пароль должен содержать не менее 4 символов");
        string name = inputString("ФИО: ");
        string phone = inputString("Телефон (+375XXXXXXXXX): ");
        string address = inputString("Адрес доставки: ");

        users.add(make_shared<Client>(login, password, name, phone, address));
        saveAll();
        cout << "Регистрация прошла успешно. Теперь вы можете войти.\n";
    }


    void DeliverySystem::clientMenu() {
        while (true) {
            cout << "\n---------- МЕНЮ КЛИЕНТА ----------\n";
            cout << "1. Список ресторанов\n";
            cout << "2. Все блюда\n";
            cout << "3. Поиск блюда по названию\n";
            cout << "4. Сортировка блюд\n";
            cout << "5. Фильтрация блюд\n";
            cout << "6. Оформить заказ\n";
            cout << "7. Мои заказы\n";
            cout << "8. Отменить заказ\n";
            cout << "0. Выйти из аккаунта\n";
            int choice = inputNumber("Ваш выбор: ", 0, 8);
            if (choice == 0)
                return;
            try {
                switch (choice) {
                case 1: showRestaurants(); break;
                case 2: showDishes(dishes.getAll()); break;
                case 3: searchDishes(); break;
                case 4: sortDishes(); break;
                case 5: filterDishes(); break;
                case 6: makeOrder(); break;
                case 7: showMyOrders(); break;
                case 8: cancelOrder(); break;
                }
            }
            catch (const DeliveryException& e) {
                cout << e.what() << endl;
            }
        }
    }


    void DeliverySystem::courierMenu() {
        while (true) {
            cout << "\n---------- МЕНЮ КУРЬЕРА ----------\n";
            cout << "1. Заказы, ожидающие доставки\n";
            cout << "2. Взять следующий заказ\n";
            cout << "3. Отметить заказ доставленным\n";
            cout << "4. Мои доставки\n";
            cout << "0. Выйти из аккаунта\n";
            int choice = inputNumber("Ваш выбор: ", 0, 4);
            if (choice == 0)
                return;
            try {
                switch (choice) {
                case 1: showWaitingOrders(); break;
                case 2: takeOrder(); break;
                case 3: finishOrder(); break;
                case 4: showMyDeliveries(); break;
                }
            }
            catch (const DeliveryException& e) {
                cout << e.what() << endl;
            }
        }
    }

  
    void DeliverySystem::adminMenu() {
        while (true) {
            cout << "\n---------- МЕНЮ АДМИНИСТРАТОРА ----------\n";
            cout << " 1. Список пользователей\n";
            cout << " 2. Добавить курьера или администратора\n";
            cout << " 3. Удалить пользователя\n";
            cout << " 4. Список ресторанов\n";
            cout << " 5. Добавить ресторан\n";
            cout << " 6. Редактировать ресторан\n";
            cout << " 7. Удалить ресторан\n";
            cout << " 8. Все блюда\n";
            cout << " 9. Добавить блюдо\n";
            cout << "10. Редактировать блюдо\n";
            cout << "11. Удалить блюдо\n";
            cout << "12. Поиск блюда\n";
            cout << "13. Сортировка блюд\n";
            cout << "14. Фильтрация блюд\n";
            cout << "15. Заказы (с фильтром по статусу)\n";
            cout << "16. Отчёт о выручке ресторанов\n";
            cout << " 0. Выйти из аккаунта\n";
            int choice = inputNumber("Ваш выбор: ", 0, 16);
            if (choice == 0)
                return;
            try {
                switch (choice) {
                case 1:  showUsers(); break;
                case 2:  addEmployee(); break;
                case 3:  deleteUser(); break;
                case 4:  showRestaurants(); break;
                case 5:  addRestaurant(); break;
                case 6:  editRestaurant(); break;
                case 7:  deleteRestaurant(); break;
                case 8:  showDishes(dishes.getAll()); break;
                case 9:  addDish(); break;
                case 10: editDish(); break;
                case 11: deleteDish(); break;
                case 12: searchDishes(); break;
                case 13: sortDishes(); break;
                case 14: filterDishes(); break;
                case 15: showAllOrders(); break;
                case 16: revenueReport(); break;
                }
            }
            catch (const DeliveryException& e) {
                cout << e.what() << endl;
            }
        }
    }



    string DeliverySystem::restaurantName(int id) {
        try {
            return restaurants.find(id)->getName();
        }
        catch (const NotFoundException&) {
            return "(удалён)";
        }
    }

    void DeliverySystem::showRestaurants() {
        cout << "\n" << setw(5) << "ID" << setw(22) << "Название" << setw(15) << "Телефон" << "Адрес\n";
        for (int i = 0; i < restaurants.size(); i++) {
            restaurants.getAll()[i]->print();
            cout << endl;
        }
    }

    void DeliverySystem::showDishes(const vector<shared_ptr<Dish>>& list) {
        if (list.empty()) {
            cout << "Ничего не найдено.\n";
            return;
        }
        cout << "\n" << setw(5) << "ID" << setw(24) << "Блюдо" << setw(14) << "Категория"
            << setw(10) << "Цена" << "Ресторан\n";
        for (size_t i = 0; i < list.size(); i++) {
            cout << *list[i] << restaurantName(list[i]->getRestaurantId()) << endl;
        }
        cout << "Всего: " << list.size() << endl;
    }

    void DeliverySystem::searchDishes() {
        string text = inputString("Введите часть названия блюда: ");
        showDishes(dishes.find(text));
    }

    void DeliverySystem::sortDishes() {
        cout << "1. По цене\n2. По названию\n";
        int choice = inputNumber("Ваш выбор: ", 1, 2);
        if (choice == 1)
            showDishes(dishes.sorted(cheaperThan));
        else
            showDishes(dishes.sorted(byName));
    }

    void DeliverySystem::filterDishes() {
        cout << "1. Блюда одного ресторана\n2. Блюда в диапазоне цен\n";
        int choice = inputNumber("Ваш выбор: ", 1, 2);
        if (choice == 1) {
            showRestaurants();
            int id = inputNumber("ID ресторана: ", 1, 1000000);
            restaurants.find(id);       
            showDishes(dishes.filter(FromRestaurant(id)));
        }
        else {
            double min = inputNumber("Цена от: ", 0.0, 10000.0);
            double max = inputNumber("Цена до: ", min, 10000.0);
            showDishes(dishes.filter(PriceRange(min, max)));
        }
    }

   

    void DeliverySystem::makeOrder() {
        shared_ptr<Client> client = dynamic_pointer_cast<Client>(currentUser);

        showRestaurants();
        int restaurantId = inputNumber("ID ресторана (0 - отмена): ", 0, 1000000);
        if (restaurantId == 0)
            return;
        shared_ptr<Restaurant> restaurant = restaurants.find(restaurantId);
        vector<shared_ptr<Dish>> menu = dishes.filter(FromRestaurant(restaurantId));
        if (menu.empty()) {
            cout << "В меню этого ресторана пока нет блюд.\n";
            return;
        }

        // адрес доставки
        string address = client->getAddress();
        cout << "Ваш адрес: " << address << endl;
        cout << "1. Доставить по этому адресу\n2. Указать другой адрес\n";
        if (inputNumber("Ваш выбор: ", 1, 2) == 2)
            address = inputString("Адрес доставки: ");

        Order order(client->getId(), restaurantId, address);

        // выбор блюд
        showDishes(menu);
        while (true) {
            int dishId = inputNumber("ID блюда (0 - закончить выбор): ", 0, 1000000);
            if (dishId == 0)
                break;
            try {
                shared_ptr<Dish> dish = dishes.find(dishId);
                int quantity = inputNumber("Количество: ", 1, 50);
                order.addItem(*dish, quantity);
                cout << "Добавлено. Сумма заказа: " << order.getSum() << " BYN\n";
            }
            catch (const DeliveryException& e) {
                cout << e.what() << endl;
            }
        }
        if (order.isEmpty()) {
            cout << "Заказ пуст - оформление отменено.\n";
            return;
        }

        order.printCheck(cout, restaurant->getName());
        cout << "1. Подтвердить заказ\n0. Отменить\n";
        if (inputNumber("Ваш выбор: ", 0, 1) == 0) {
            cout << "Заказ отменён.\n";
            return;
        }

        shared_ptr<Order> newOrder = make_shared<Order>(order);
        orders.add(newOrder);
        saveAll();

        // чек дописывается в конец файла 
        ofstream fout("checks.txt", ios::out | ios::app);
        if (fout.is_open()) {
            fout << fixed << setprecision(2);
            newOrder->printCheck(fout, restaurant->getName());
            fout.close();
        }
        cout << "Заказ № " << newOrder->getId() << " оформлен! Чек сохранён в файл checks.txt\n";
    }

    void DeliverySystem::showOrders(const vector<shared_ptr<Order>>& list) {
        if (list.empty()) {
            cout << "Заказов нет.\n";
            return;
        }
        cout << "\n" << setw(6) << "№" << setw(12) << "Статус" << setw(10) << "Сумма" << "Адрес\n";
        for (size_t i = 0; i < list.size(); i++) {
            list[i]->print();
            cout << endl;
        }
    }

    void DeliverySystem::showMyOrders() {
        vector<shared_ptr<Order>> myOrders;
        for (int i = 0; i < orders.size(); i++)
            if (orders.getAll()[i]->getClientId() == currentUser->getId())
                myOrders.push_back(orders.getAll()[i]);
        showOrders(myOrders);
        if (myOrders.empty())
            return;

        int id = inputNumber("Номер заказа для просмотра чека (0 - назад): ", 0, 1000000);
        if (id == 0)
            return;
        shared_ptr<Order> order = orders.find(id);
        if (order->getClientId() != currentUser->getId())
            throw DeliveryException("Это не ваш заказ.");
        order->printCheck(cout, restaurantName(order->getRestaurantId()));
    }

    void DeliverySystem::cancelOrder() {
        int id = inputNumber("Номер заказа для отмены: ", 1, 1000000);
        shared_ptr<Order> order = orders.find(id);
        if (order->getClientId() != currentUser->getId())
            throw DeliveryException("Можно отменить только свой заказ.");
        order->setStatus(CANCELLED);     // исключение, если заказ уже в пути
        saveAll();
        cout << "Заказ № " << id << " отменён.\n";
    }

 

    void DeliverySystem::showWaitingOrders() {
        showOrders(orders.filter(WithStatus(WAITING)));
    }

    // Курьер берёт заказ, который ждёт дольше всех 
    void DeliverySystem::takeOrder() {
        shared_ptr<Courier> courier = dynamic_pointer_cast<Courier>(currentUser);
        if (courier->isBusy())
            throw DeliveryException("Сначала завершите текущую доставку.");

        queue<shared_ptr<Order>> waiting;
        for (int i = 0; i < orders.size(); i++)
            if (orders.getAll()[i]->getStatus() == WAITING)
                waiting.push(orders.getAll()[i]);
        if (waiting.empty()) {
            cout << "Заказов, ожидающих доставки, нет.\n";
            return;
        }

        shared_ptr<Order> order = waiting.front();
        order->setCourier(courier->getId());
        order->setStatus(DELIVERING);
        courier->setBusy(true);
        saveAll();
        cout << "Вы взяли заказ № " << order->getId() << ". В очереди осталось: "
            << waiting.size() - 1 << endl;
        order->printCheck(cout, restaurantName(order->getRestaurantId()));
    }

    void DeliverySystem::finishOrder() {
        shared_ptr<Courier> courier = dynamic_pointer_cast<Courier>(currentUser);
        for (int i = 0; i < orders.size(); i++) {
            shared_ptr<Order> order = orders.getAll()[i];
            if (order->getCourierId() == courier->getId() && order->getStatus() == DELIVERING) {
                order->setStatus(DELIVERED);
                courier->addDelivery();
                courier->setBusy(false);
                saveAll();
                cout << "Заказ № " << order->getId() << " доставлен.\n";
                return;
            }
        }
        cout << "У вас нет заказов в пути.\n";
    }

    void DeliverySystem::showMyDeliveries() {
        vector<shared_ptr<Order>> list;
        for (int i = 0; i < orders.size(); i++)
            if (orders.getAll()[i]->getCourierId() == currentUser->getId())
                list.push_back(orders.getAll()[i]);
        showOrders(list);
    }

    

    void DeliverySystem::showUsers() {
        cout << "\n" << setw(5) << "ID" << setw(12) << "Логин" << setw(28) << "ФИО"
            << setw(15) << "Телефон" << setw(15) << "Роль" << "Доп. сведения\n";
        for (int i = 0; i < users.size(); i++) {
            users.getAll()[i]->print();    // полиморфный вызов
            cout << endl;
        }
    }

    void DeliverySystem::addEmployee() {
        cout << "1. Курьер\n2. Администратор\n";
        int role = inputNumber("Роль: ", 1, 2);
        string login = inputString("Логин: ");
        if (loginExists(login))
            throw InputException("логин \"" + login + "\" уже занят");
        string password = inputPassword("Пароль: ");
        string name = inputString("ФИО: ");
        string phone = inputString("Телефон (+375XXXXXXXXX): ");

        if (role == 1)
            users.add(make_shared<Courier>(login, password, name, phone));
        else
            users.add(make_shared<Administrator>(login, password, name, phone));
        saveAll();
        cout << "Пользователь добавлен.\n";
    }

    void DeliverySystem::deleteUser() {
        showUsers();
        int id = inputNumber("ID пользователя для удаления: ", 1, 1000000);
        if (id == currentUser->getId())
            throw DeliveryException("Нельзя удалить самого себя.");
        users.remove(id);
        saveAll();
        cout << "Пользователь удалён.\n";
    }

    void DeliverySystem::addRestaurant() {
        string name = inputString("Название: ");
        string address = inputString("Адрес: ");
        string phone = inputString("Телефон (+375XXXXXXXXX): ");
        restaurants.add(make_shared<Restaurant>(name, address, phone));
        saveAll();
        cout << "Ресторан добавлен.\n";
    }

    void DeliverySystem::editRestaurant() {
        showRestaurants();
        shared_ptr<Restaurant> restaurant = restaurants.find(inputNumber("ID ресторана: ", 1, 1000000));
        cout << "1. Название\n2. Адрес\n3. Телефон\n";
        int choice = inputNumber("Что изменить: ", 1, 3);
        string value = inputString("Новое значение: ");
        if (choice == 1)
            restaurant->setName(value);
        else if (choice == 2)
            restaurant->setAddress(value);
        else
            restaurant->setPhone(value);
        saveAll();
        cout << "Ресторан изменён.\n";
    }

    void DeliverySystem::deleteRestaurant() {
        showRestaurants();
        int id = inputNumber("ID ресторана для удаления: ", 1, 1000000);
        restaurants.remove(id);
        // удаляем и все блюда этого ресторана
        vector<shared_ptr<Dish>> menu = dishes.filter(FromRestaurant(id));
        for (size_t i = 0; i < menu.size(); i++)
            dishes.remove(menu[i]->getId());
        saveAll();
        cout << "Ресторан и его блюда удалены.\n";
    }

    void DeliverySystem::addDish() {
        showRestaurants();
        int restaurantId = inputNumber("ID ресторана: ", 1, 1000000);
        restaurants.find(restaurantId);
        string name = inputString("Название блюда: ");
        string category = inputString("Категория: ");
        double price = inputNumber("Цена: ", 0.01, 10000.0);
        dishes.add(make_shared<Dish>(name, category, price, restaurantId));
        saveAll();
        cout << "Блюдо добавлено.\n";
    }

    void DeliverySystem::editDish() {
        showDishes(dishes.getAll());
        shared_ptr<Dish> dish = dishes.find(inputNumber("ID блюда: ", 1, 1000000));
        cout << "1. Название\n2. Категория\n3. Цена\n";
        int choice = inputNumber("Что изменить: ", 1, 3);
        if (choice == 1)
            dish->setName(inputString("Новое название: "));
        else if (choice == 2)
            dish->setCategory(inputString("Новая категория: "));
        else
            dish->setPrice(inputNumber("Новая цена: ", 0.01, 10000.0));
        saveAll();
        cout << "Блюдо изменено.\n";
    }

    void DeliverySystem::deleteDish() {
        showDishes(dishes.getAll());
        dishes.remove(inputNumber("ID блюда для удаления: ", 1, 1000000));
        saveAll();
        cout << "Блюдо удалено.\n";
    }

    void DeliverySystem::showAllOrders() {
        cout << "0. Все заказы\n1. Ожидают\n2. В пути\n3. Доставлены\n4. Отменены\n";
        int choice = inputNumber("Ваш выбор: ", 0, 4);
        if (choice == 0)
            showOrders(orders.getAll());
        else
            showOrders(orders.filter(WithStatus(choice - 1)));
    }

    // Отчёт о выручке ресторанов по доставленным заказам
    void DeliverySystem::revenueReport() {
        int n = restaurants.size();
        if (n == 0) {
            cout << "Ресторанов нет.\n";
            return;
        }
        RevenueReport report(n);    // объект с динамическими массивами
        for (int i = 0; i < n; i++)
            report.setName(i, restaurants.getAll()[i]->getName());

        for (int j = 0; j < orders.size(); j++) {
            shared_ptr<Order> order = orders.getAll()[j];
            if (order->getStatus() != DELIVERED)
                continue;
            for (int i = 0; i < n; i++)
                if (restaurants.getAll()[i]->getId() == order->getRestaurantId())
                    report.addOrder(i, order->getSum());
        }

        cout << "\n" << report;          
        ofstream fout("report.txt");
        if (!fout.is_open())
            throw FileException("не удалось создать файл report.txt");
        fout << report;
        if (fout.fail())
            throw FileException("ошибка записи в файл report.txt");
        fout.close();
        cout << "Отчёт сохранён в файл report.txt\n";
    }

}