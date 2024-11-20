#include <lib/ArgParser/arg_parser.h>

#include <iostream>
#include <numeric>

struct Options {
    bool& sum;
    bool& mult;
};

int main(int argc, char** argv) {
    ArgumentParser::ArgParser parser("Program");

    std::vector<float>& values = parser.AddArgument<float, 1>("N").Positional().GetStorage();

    Options opt{parser.AddFlag("sum", "add args").GetStorage(),
                parser.AddFlag("mult", "multiply args").GetStorage()};

    parser.AddHelp("Program accumulate arguments");

    if (!parser.Parse(argc, argv)) {
        std::cout << "Wrong argument" << std::endl;
        std::cout << parser.HelpDescription() << std::endl;
        return 1;
    }

    if (parser.Help()) {
        std::cout << parser.HelpDescription() << std::endl;
        return 0;
    }

    if (opt.sum) {
        std::cout << "Result: " << std::accumulate(values.begin(), values.end(), 0.0) << std::endl;
    } else if (opt.mult) {
        std::cout << "Result: " << std::accumulate(values.begin(), values.end(), 1, std::multiplies<int>())
                  << std::endl;
    } else {
        std::cout << "No one options had chosen" << std::endl;
        std::cout << parser.HelpDescription();
        return 1;
    }

    return 0;
}
