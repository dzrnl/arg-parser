#include <lib/ArgParser/arg_parser.h>

#include <gtest/gtest.h>
#include <sstream>

using namespace ArgumentParser;

std::vector<std::string> SplitString(const std::string& str) {
    std::istringstream iss(str);

    return {std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>()};
}


TEST(ArgParserTestSuite, EmptyTest) {
    ArgParser parser("My Empty Parser");

    ASSERT_TRUE(parser.Parse(SplitString("app")));
}


TEST(ArgParserTestSuite, StringTest) {
    ArgParser parser("My Parser");
    parser.AddArgument<std::string>("param1");

    ASSERT_TRUE(parser.Parse(SplitString("app --param1=value1")));
    ASSERT_EQ(parser.GetArgumentValue<std::string>("param1"), "value1");
}


TEST(ArgParserTestSuite, ShortNameTest) {
    ArgParser parser("My Parser");
    parser.AddArgument<std::string>('p', "param1");

    ASSERT_TRUE(parser.Parse(SplitString("app -p value1")));
    ASSERT_EQ(parser.GetArgumentValue<std::string>("param1"), "value1");
}


TEST(ArgParserTestSuite, DefaultTest) {
    ArgParser parser("My Parser");
    parser.AddArgument<std::string>("param1").Default("value1");

    ASSERT_TRUE(parser.Parse(SplitString("app")));
    ASSERT_EQ(parser.GetArgumentValue<std::string>("param1"), "value1");
}


TEST(ArgParserTestSuite, NoDefaultTest) {
    ArgParser parser("My Parser");
    parser.AddArgument<std::string>("param1");

    ASSERT_FALSE(parser.Parse(SplitString("app")));
}


TEST(ArgParserTestSuite, StoreValueTest) {
    ArgParser parser("My Parser");
    std::string& value = parser.AddArgument<std::string>("param1").GetStorage();

    ASSERT_TRUE(parser.Parse(SplitString("app --param1=value1")));
    ASSERT_EQ(value, "value1");
}


TEST(ArgParserTestSuite, MultiStringTest) {
    ArgParser parser("My Parser");
    parser.AddArgument<std::string>("param1");
    parser.AddArgument<std::string>('a', "param2");

    ASSERT_TRUE(parser.Parse(SplitString("app --param1=value1 --param2=value2")));
    ASSERT_EQ(parser.GetArgumentValue<std::string>("param2"), "value2");
}


TEST(ArgParserTestSuite, IntTest) {
    ArgParser parser("My Parser");
    parser.AddArgument<int>("param1");

    ASSERT_TRUE(parser.Parse(SplitString("app --param1=100500")));
    ASSERT_EQ(parser.GetArgumentValue<int>("param1"), 100500);
}


TEST(ArgParserTestSuite, MultiValueTest) {
    ArgParser parser("My Parser");
    std::vector<int>& int_values = parser.AddArgument<int, 1>('p', "param1").GetStorage();

    ASSERT_TRUE(parser.Parse(SplitString("app --param1=1 --param1=2 --param1=3")));
    ASSERT_EQ(parser.GetArgumentValue<int>("param1", 0), 1);
    ASSERT_EQ(int_values[1], 2);
    ASSERT_EQ(int_values[2], 3);
}


TEST(ArgParserTestSuite, MinCountMultiValueTest) {
    ArgParser parser("My Parser");
    parser.AddArgument<int, 10>('p', "param1");

    ASSERT_FALSE(parser.Parse(SplitString("app --param1=1 --param1=2 --param1=3")));
}


TEST(ArgParserTestSuite, FlagTest) {
    ArgParser parser("My Parser");
    parser.AddFlag('f', "flag1");

    ASSERT_TRUE(parser.Parse(SplitString("app --flag1")));
    ASSERT_TRUE(parser.GetArgumentValue<bool>("flag1"));
}


TEST(ArgParserTestSuite, FlagsTest) {
    ArgParser parser("My Parser");
    parser.AddFlag('a', "flag1");
    parser.AddFlag('b', "flag2").Default(true);
    bool& flag3 = parser.AddFlag('c', "flag3").GetStorage();

    ASSERT_TRUE(parser.Parse(SplitString("app -ac")));
    ASSERT_TRUE(parser.GetArgumentValue<bool>("flag1"));
    ASSERT_TRUE(parser.GetArgumentValue<bool>("flag2"));
    ASSERT_TRUE(flag3);
}


TEST(ArgParserTestSuite, PositionalArgTest) {
    ArgParser parser("My Parser");
    std::vector<int>& values = parser.AddArgument<int, 1>("Param1").Positional().GetStorage();

    ASSERT_TRUE(parser.Parse(SplitString("app 1 2 3 4 5")));
    ASSERT_EQ(values[0], 1);
    ASSERT_EQ(values[2], 3);
    ASSERT_EQ(values.size(), 5);
}


TEST(ArgParserTestSuite, HelpTest) {
    ArgParser parser("My Parser");
    std::vector<int>& values = parser.AddArgument<int, 1>("Param1").Positional().GetStorage();

    ASSERT_TRUE(parser.Parse(SplitString("app 1 2 3 4 5")));
    ASSERT_EQ(values[0], 1);
    ASSERT_EQ(values[2], 3);
    ASSERT_EQ(values.size(), 5);
}

struct SomeStruct {
    int a;
};

std::istream& operator>>(std::istream& is, SomeStruct& someStruct) {
    is >> someStruct.a;

    return is;
}

std::ostream& operator<<(std::ostream& os, const SomeStruct& someStruct) {
    os << someStruct.a;

    return os;
}

TEST(ArgParserTestSuite, MyTypeTest) {
    ArgParser parser("My Parser");
    parser.AddArgument<SomeStruct>("param1");

    ASSERT_TRUE(parser.Parse(SplitString("app --param1=100500")));
    ASSERT_EQ(parser.GetArgumentValue<SomeStruct>("param1").a, 100500);
}


TEST(ArgParserTestSuite, HelpStringTest) {
    ArgParser parser("My Parser");
    parser.AddHelp("Some Description about program");
    parser.AddArgument<std::string, 1>('i', "input", "File path for input file");
    parser.AddFlag('s', "flag1", "Use some logic").Default(true);
    parser.AddFlag('p', "flag2", "Use some logic");
    parser.AddArgument<int>("number", "Some Number");

    ASSERT_FALSE(parser.Parse(SplitString("app --help")));

    // Проверка закомментирована намеренно. Ожидается, что результат вызова функции будет приблизительно такой же,
    // но не с точностью до символа

    // ASSERT_EQ(
    //     parser.HelpDescription(),
    //     "My Parser\n"
    //     "Some Description about program\n"
    //     "\n"
    //     "-i,  --input=<string>,  File path for input file [repeated, min args = 1]\n"
    //     "-s,  --flag1,  Use some logic [default = true]\n"
    //     "-p,  --flag2,  Use some logic\n"
    //     "     --number=<int>,  Some Number\n"
    //     "\n"
    //     "-h, --help Display this help and exit\n"
    // );
}
