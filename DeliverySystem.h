#pragma once
#pragma once
#include "Storage.h"

namespace Delivery {

    class DeliverySystem {
        Storage<User> users;
        Storage<Restaurant> restaurants;
        Storage<Dish> dishes;
        Storage<Order> orders;
        shared_ptr<User> currentUser;  // кто сейчас вошёл в систему

        DeliverySystem();              // закрытый конструктор 

        // работа с файлами
        void loadAll();
        void saveAll();
        void createDemoData();

        // вход и регистрация
        void login();
        void registerClient();
        bool loginExists(const string& login);

        // меню пользователей
        void clientMenu();
        void courierMenu();
        void adminMenu();

        // рестораны и блюда (общие функции)
        void showRestaurants();
        void showDishes(const vector<shared_ptr<Dish>>& list);
        void searchDishes();
        void sortDishes();
        void filterDishes();
        string restaurantName(int id);

        // функции клиента
        void makeOrder();
        void showMyOrders();
        void cancelOrder();

        // функции курьера
        void showWaitingOrders();
        void takeOrder();
        void finishOrder();
        void showMyDeliveries();

        // функции администратора
        void showUsers();
        void addEmployee();
        void deleteUser();
        void addRestaurant();
        void editRestaurant();
        void deleteRestaurant();
        void addDish();
        void editDish();
        void deleteDish();
        void showAllOrders();
        void revenueReport();

        void showOrders(const vector<shared_ptr<Order>>& list);

    public:
        DeliverySystem(const DeliverySystem&) = delete;            // копирование запрещено
        DeliverySystem& operator=(const DeliverySystem&) = delete;

        static DeliverySystem& getInstance();   // единственный объект системы
        void run();
    };

}
