// Orbyte_Prototype.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>

class satellite
{
    public:
        int altitude = 0;
        std::string ID;

        satellite(int _altitude, std::string _ID) {
            altitude = _altitude;
            ID = _ID;
        }

        int get_altitude()
        {
            return altitude;
        }

        int Update(float delta)
        {
            //There'll be a good reason for having delta as a parameter... I dont know what that is just yet
            return 0;
        }

        void Debug()
        {
            std::cout << ID;

        }
};

int mainloop()
{
    while (1)
    {
        //The observant among you will notice that this should execute infinitely!
        
    }
}

int main()
{
    std::cout << "Orbyte Prototype\n";

    satellite test(
        12
    );
    int test_out = test.get_altitude();

    std::cout << test_out;

    mainloop();

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
