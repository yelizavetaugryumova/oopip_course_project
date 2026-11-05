#pragma once
#pragma once
#include <fstream>
#include <algorithm>
#include "Classes.h"

namespace Delivery {

    template <class T>
    class Storage {
        vector<shared_ptr<T>> items;   // записи в оперативной памяти
        string fileName;               // файл для хранения записей
    public:
        Storage(const string& fileName) : fileName(fileName)
        {}

        void add(shared_ptr<T> item) { 
            items.push_back(item);
        }

        const vector<shared_ptr<T>>& getAll() const {
            return items; 
        }
        int size() const {
            return items.size(); 
        }

        // удаление записи по номеру
        void remove(int id) {
            typename vector<shared_ptr<T>>::iterator it;
            for (it = items.begin(); it != items.end(); it++) {
                if ((*it)->getId() == id) {
                    items.erase(it);
                    return;
                }
            }
            throw NotFoundException("запись с ID " + to_string(id));
        }

        //перегрузка методов: поиск по номеру
        shared_ptr<T> find(int id) const {
            for (size_t i = 0; i < items.size(); i++)
                if (items[i]->getId() == id)
                    return items[i];
            throw NotFoundException("запись с ID " + to_string(id));
        }

        // поиск по части названия без учёта регистра
        vector<shared_ptr<T>> find(const string& text) const {
            vector<shared_ptr<T>> result;
            for (size_t i = 0; i < items.size(); i++)
                if (toLower(items[i]->getName()).find(toLower(text)) != string::npos)
                    result.push_back(items[i]);
            return result;
        }

        //  отбор записей по условию 
        template <class Condition>
        vector<shared_ptr<T>> filter(Condition condition) const {
            vector<shared_ptr<T>> result;
            for (size_t i = 0; i < items.size(); i++)
                if (condition(items[i]))
                    result.push_back(items[i]);
            return result;
        }

        // шаблон метода: отсортированная копия списка (исходный порядок не меняется)
        template <class Compare>
        vector<shared_ptr<T>> sorted(Compare compare) const {
            vector<shared_ptr<T>> result = items;
            sort(result.begin(), result.end(), compare);
            return result;
        }

        // Чтение записей из файла
        void load() {
            ifstream fin(fileName);
            if (!fin.is_open())     // файла ещё нет - первый запуск программы
                return;
            items.clear();
            string line;
            int lineNumber = 0;
            while (getline(fin, line)) {
                lineNumber++;
                if (line.empty())
                    continue;
                try {
                    items.push_back(T::fromLine(line));
                }
                catch (const exception&) {
                    fin.close();
                    throw FileException("повреждена строка " + to_string(lineNumber)
                        + " в файле " + fileName);
                }
            }
            fin.close();
        }

        // Запись всех записей в файл
        void save() const {
            ofstream fout(fileName);
            if (!fout.is_open())
                throw FileException("не удалось открыть файл " + fileName);
            for (size_t i = 0; i < items.size(); i++)
                fout << items[i]->toLine() << endl;
            if (fout.fail())        // контроль ошибок записи
                throw FileException("ошибка записи в файл " + fileName);
            fout.close();
        }
    };

}
