// Решите загадку: Сколько чисел от 1 до 1000 содержат как минимум одну цифру 3?
// Напишите ответ здесь:
#include <iostream>

using namespace std;

int CountingTrip()
{
    int Number = 0;
    for (int i = 1; i <= 1000; ++i)
    {

        if ((i / 1000) % 10 == 3 || (i / 100) % 10 == 3 || (i / 10) % 10 == 3 || (i / 1) % 10 == 3)
        {
            ++Number;
        }

    }
    return Number;
}

int main() {
    int NumTrip = CountingTrip();
    cout << NumTrip;
}

// Закомитьте изменения и отправьте их в свой репозиторий.
