#include <catch/catch.hpp>

#include <world/core.h>

using namespace world;

TEST_CASE("ItemKeys", "[collector]") {
    ItemKey key1 = ItemKeys::root("0");
    ItemKey key2 = ItemKeys::root("0");
    ItemKey key3 = ItemKeys::root("1");

    ItemKey keyc1 = ItemKeys::child(key1, "51");
    ItemKey keyc2 = ItemKeys::child(key2, "51");
    ItemKey keyc3 = ItemKeys::child(key3, "51");
    ItemKey keyc4 = ItemKeys::child(key1, "52");

    SECTION("Keys comparison") {
        CHECK(key1 == key2);
        CHECK_FALSE(key1 < key2);
        CHECK_FALSE(key2 < key1);

        CHECK_FALSE(key3 == key1);
        CHECK((key1 < key3) != (key3 < key1));

        CHECK(keyc1 == keyc2);
        CHECK_FALSE(keyc1 < keyc2);
        CHECK_FALSE(keyc2 < keyc1);

        CHECK_FALSE(keyc3 == keyc1);
        CHECK((keyc1 < keyc3) != (keyc3 < keyc1));
        CHECK_FALSE(keyc4 == keyc1);
        CHECK((keyc1 < keyc4) != (keyc4 < keyc1));
    }

    SECTION("Concat") {
        ItemKey test = ItemKeys::child(ItemKeys::child(keyc1, "1"), "51");
        CHECK(test == ItemKeys::concat(keyc1, keyc3));
    }

    SECTION("toString / fromString") {
        ItemKey keystr = key(ItemKeys::toString(key1));
        CHECK(key1 == keystr);
        CHECK(ItemKeys::toString(key1) == ItemKeys::toString(keystr));

        CHECK(keyc1 == key(ItemKeys::toString(keyc1)));

        CHECK_FALSE(ItemKeys::toString(key1) == ItemKeys::toString(key3));
        CHECK_FALSE(ItemKeys::toString(keyc1) == ItemKeys::toString(keyc3));
    }
}

TEST_CASE("ExplorationContext", "[collector]") {
    ExplorationContext ctx = ExplorationContext::getDefault();

    SECTION("default context") {
        CHECK(ctx.mutateKey(ItemKeys::root("a")) == ItemKeys::root("a"));
        CHECK((ctx.getOffset() - vec3d{0, 0, 0}).norm() == Approx(0));
    }

    SECTION("context with parameters") {
        ctx.appendPrefix("a");
        ctx.appendPrefix("b");
        ctx.addOffset({1, 1, 1});
        ctx.addOffset({4, 5, 6});

        CHECK((ctx.getOffset() - vec3d{5, 6, 7}).norm() == Approx(0));
        auto key = ctx.mutateKey(ItemKeys::root("c"));
        CHECK(key == ItemKeys::child(ItemKeys::child(ItemKeys::root("a"), "b"), "c"));
    }
}

TEST_CASE("Collector", "[collector]") {
    Collector collector;

    SECTION("adding channels") {
        CHECK_FALSE(collector.hasChannel<Object3D>());
        CHECK_FALSE(collector.hasStorageChannel<Object3D>());

        collector.addStorageChannel<Object3D>();

        CHECK(collector.hasChannel<Object3D>());
        CHECK(collector.hasStorageChannel<Object3D>());
    }

    auto &objChan = collector.addStorageChannel<Object3D>();
    auto &matChan = collector.addStorageChannel<Material>();

    SECTION("adding object") {
        auto key = ItemKeys::root("a");
        Material mat01("mat01");

        CHECK_FALSE(matChan.has(key));
        CHECK_THROWS(matChan.get(key));

        matChan.put(key, mat01);
        CHECK(matChan.has(key));
        CHECK_NOTHROW(matChan.get(key));
        CHECK(matChan.get(key).getName() == "mat01");

        matChan.remove(key);
        CHECK_FALSE(matChan.has(key));
        CHECK_THROWS(matChan.get(key));
    }

    SECTION("using Context") {
        ExplorationContext context;
        context.appendPrefix("a");
        context.addOffset({5, 6, 7.3});

        ItemKey key = ItemKeys::root("b");
        ItemKey ctxKey = ItemKeys::concat(ItemKeys::root("a"), key);
        Object3D object;

        SECTION("key modification") {
            objChan.put(key, object, context);
            CHECK(objChan.has(key, context));
            CHECK(objChan.has(ctxKey));
        }

        SECTION("position modification") {
            objChan.put(key, object, context);
            CHECK((objChan.get(ctxKey).getPosition() - (object.getPosition() + vec3d{5, 6, 7.3})).norm() == Approx(0));
        }
    }
}
