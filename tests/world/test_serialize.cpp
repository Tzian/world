#include <catch/catch.hpp>

#include <world/core.h>

using namespace world;


TEST_CASE("WorldFile", "[serialize]") {

    WorldFile wf;
    wf.addString("1", "1s");
    wf.addInt("2", 1);
    wf.addDouble("3", 3.2);
    wf.addBool("4", true);

    WorldFile a1, a2, a3, a4;

    a1.addString("0", "0s");
    a2.addString("0", "1s");

    wf.addArray("6");
    wf.addToArray("6", std::move(a1));
    wf.addToArray("6", std::move(a2));

    WorldFile c1;
    c1.addString("0", "s");
    wf.addChild("c1", std::move(c1));

    SECTION("Test read") {
        CHECK(wf.readString("1") == "1s");
        CHECK(wf.readInt("2") == 1);
        CHECK(wf.readFloat("3") == Approx(3.2));
        CHECK(wf.readDouble("3") == Approx(3.2));
        CHECK(wf.readBool("4"));

        CHECK_THROWS(wf.readString("2"));
        CHECK_THROWS(wf.readFloat("2"));
        CHECK_THROWS(wf.readInt("3"));
        CHECK_THROWS(wf.readString("100"));
    }

    SECTION("Test read objects") {
        auto &rc1 = wf.readChild("c1");
        CHECK(rc1.readString("0") == "s");

        SECTION("Try to reread objects") {
            auto &rc1bis = wf.readChild("c1");
            CHECK(rc1bis.readString("0") == "s");
        }
    }

    SECTION("Test read array") {
        std::vector<std::string> results;
        for (auto it = wf.readArray("6"); !it.end(); ++it) {
            results.push_back(it->readString("0"));
        }
        CHECK(results == std::vector<std::string>{"0s", "1s"});
    }

    SECTION("Test jsonify") {
        WorldFile wf2;
        wf2.fromJson(wf.toJson());

        CHECK(wf2.readString("1") == "1s");
        CHECK(wf2.readInt("2") == 1);
        CHECK(wf2.readFloat("3") == Approx(3.2));
        CHECK(wf2.readBool("4"));
    }

    SECTION("Test read and save") {
        world::createDirectories("unittests");

        std::string filename("unittests/wf_test.json");
        wf.save(filename);

        WorldFile wf2;
        wf2.load(filename);

        CHECK(wf2.readString("1") == "1s");
        CHECK(wf2.readInt("2") == 1);
        CHECK(wf2.readFloat("3") == Approx(3.2));
        CHECK(wf2.readBool("4"));
    }

    SECTION("Move ctor") {
        WorldFile wfc(std::move(wf));

        CHECK(wfc.readBool("4"));
    }

    SECTION("Complex json") {
        WorldFile wcin;
        wcin.addStruct("position", vec3d{5});
        INFO(wcin.toJson());

        WorldFile wcout;
        wcout.addArray("nodes");
        wcout.addToArray("nodes", std::move(wcin));
        CHECK(wcout.toJson() ==
              "{\"nodes\":[{\"position\":{\"x\":5.0,\"y\":5.0,\"z\":5.0}}]}");
    }
}

class BaseClass1 : public ISerializable {};

class ChildClass1 : public BaseClass1 {
    WORLD_WRITE_SUBCLASS_METHOD
};

class ChildClass2 : public BaseClass1 {
    WORLD_WRITE_SUBCLASS_METHOD
};

template <typename T> class ChildClassTemplate : public BaseClass1 {
public:
    WORLD_WRITE_SUBCLASS_METHOD
};

// The current implementation of WORLD_REGISTER_BASE_CLASS
// does not compile out of the world namespace.
namespace world {
// On Windows the symbol should not be exported
// TODO Add a WORLDAPI_ONLY_EXPORT macro to avoid __dllimport() the specialized
// templates And fix this issue
#define WORLDAPI_EXPORT
WORLD_REGISTER_BASE_CLASS(BaseClass1)
} // namespace world
WORLD_REGISTER_CHILD_CLASS(BaseClass1, ChildClass1, "1")
WORLD_REGISTER_CHILD_CLASS(BaseClass1, ChildClass2, "2")
WORLD_REGISTER_TEMPLATE_CHILD_CLASS(BaseClass1, ChildClassTemplate, int, "int")
WORLD_REGISTER_TEMPLATE_CHILD_CLASS(BaseClass1, ChildClassTemplate, double,
                                    "double")

TEST_CASE("ISerializable", "[serialize]") {
    SECTION("Polymorphism") {

        WorldFile wf;
        ChildClass1 c1;
        c1.writeSubclass(wf);

        BaseClass1 *r = readSubclass<BaseClass1>(wf);
        CHECK(dynamic_cast<ChildClass1 *>(r) != nullptr);
        CHECK(dynamic_cast<ChildClass2 *>(r) == nullptr);
    }

    SECTION("Template polymorphism") {
        WorldFile wf;
        ChildClassTemplate<double> ct;
        ct.writeSubclass(wf);

        BaseClass1 *r = readSubclass<BaseClass1>(wf);
        CHECK(dynamic_cast<ChildClass1 *>(r) == nullptr);
        CHECK(dynamic_cast<ChildClassTemplate<int> *>(r) == nullptr);
        CHECK(dynamic_cast<ChildClassTemplate<double> *>(r) != nullptr);
    }
}