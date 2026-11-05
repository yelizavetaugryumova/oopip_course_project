
//   admin / admin123 - администратор
//   courier / courier123 - курьер
//   client / client123 - клиент
#include "DeliverySystem.h"
#include <clocale>
#include <cstdlib>

int main() {
#ifdef _WIN32
    system("chcp 1251");           
    system("cls");
#endif
    setlocale(LC_CTYPE, "Russian"); // для перевода русских букв в нижний регистр при поиске
    Delivery::DeliverySystem::getInstance().run();
    return 0;
}