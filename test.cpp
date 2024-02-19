#include <sstream>
#include <vector>
#include "gtest/gtest.h"
#include "csv_reader.h"

class Person {
	private:
		std::string _name;
		unsigned int _age;

	public:
		Person() noexcept: _name("John Doe"), _age(50) {}
		Person(std::string name, unsigned int age)
			noexcept: _name(name), _age(age) {}
		~Person() = default;
		Person& operator=(const Person& other) noexcept = default;

		bool operator==(const Person& other) const noexcept {
			return this->_name == other._name && this->_age == other._age;
		}

		const std::string& name() const noexcept {
			return this->_name;
		}

		unsigned int age() const noexcept {
			return this->_age;
		}

		friend std::ostream& operator<<(std::ostream& s, const Person& p) {
			s << "Name: " << p.name() << "; Age: " << p.age();
			return s;
		}

		friend std::istream& operator>>(std::istream& s, Person& p) {
			s >> p._name >> p._age;
			return s;
		}
};

template<typename>
struct Parser;

template<>
struct Parser<int> {
	int operator()(const std::string& s) {
		return stoi(s);
	}
};

template<>
struct Parser<std::string> {
	std::string operator()(const std::string& s) {
		return s;
	}
};

template<>
struct Parser<Person> {
	Person operator()(const std::string& s) {
		Person person;
		std::istringstream stream(s);
		stream >> person;
		return person;
	}
};

template<>
struct Parser<double> {
	double operator()(const std::string& s) {
		return stod(s);
	}
};

namespace {

TEST(CsvReaderTest, all) {
	std::string content = "qwkjekwqejqwe\n-5,Hello world!,John 30,3.14,,www\n";
	std::istringstream stream(content);

	csv_reader::Reader<Parser, int, std::string, Person, double> r(stream, 1);
	for (
		std::tuple<int, std::string, Person, double, std::vector<std::string>> t : r
	) {
		EXPECT_EQ(std::get<0>(t), -5);
		EXPECT_EQ(std::get<1>(t), "Hello world!");
		EXPECT_EQ(std::get<2>(t), Person("John", 30));
		EXPECT_EQ(std::get<3>(t), 3.14);
		EXPECT_EQ(std::get<4>(t).size(), 2);
		EXPECT_EQ(std::get<4>(t)[0], "");
		EXPECT_EQ(std::get<4>(t)[1], "www");
	}
}

}
