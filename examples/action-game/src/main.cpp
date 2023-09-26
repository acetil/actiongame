#include <stdlib.h>

#include <logging/logging.h>
#include <phenyl/phenyl.h>

#include "game/test_app.h"

using namespace phenyl;

struct Foo {
    int a;
};

struct Bar : public Foo {
    float b;
};

struct Bar2 : public Foo {
    std::string s;
};

struct Baz : public Bar {
    char c;
};

struct TestBase {
    virtual int getVal () = 0;
};

struct Test : TestBase {
    int d;

    int getVal() override {
        return d * 10;
    }

    explicit Test (int d) : d{d} {}
};

struct Test1 : TestBase {
    int e;

    int getVal() override {
        return e * 1000;
    }

    explicit Test1 (int e) : e{e} {}
};

struct Test2 {
    float f;
};

static void testCompManager () {
    component::logging::log(LEVEL_DEBUG, "Starting comp manager tests!");
    auto manager = phenyl::ComponentManager{2};

    manager.addComponent<Foo>();
    manager.addComponent<Bar>();
    manager.addComponent<Bar2>();
    manager.addComponent<Baz>();
    manager.addComponent<Test>();
    manager.addComponent<Test1>();
    manager.addComponent<Test2>();
    manager.addComponent<TestBase>();

    manager.inherits<Bar, Foo>();
    manager.inherits<Bar2, Foo>();
    manager.inherits<Baz, Bar>();

    manager.inherits<Test, TestBase>();
    manager.inherits<Test1, TestBase>();

    manager.addRequirement<Test2, Test>();

    auto e1 = manager.create();
    auto e2 = manager.create();
    auto e3 = e2.createChild();
    auto e4 = e2.createChild();
    auto e5 = e2.createChild();

    component::logging::log(LEVEL_DEBUG, "E1: {}", e1.id().value());
    component::logging::log(LEVEL_DEBUG, "E2: {}", e2.id().value());
    component::logging::log(LEVEL_DEBUG, "E3: {}", e3.id().value());
    component::logging::log(LEVEL_DEBUG, "E4: {}", e4.id().value());
    component::logging::log(LEVEL_DEBUG, "E5: {}", e5.id().value());

    e1.insert<Foo>(Foo{1});
    e2.insert<Bar>(Bar{2, 1.1f});
    e3.insert<Baz>(Baz{3, 2.2f, 'f'});
    e4.insert<Bar2>(Bar2{4, "hello"});

    e5.insert<Test2>(Test2{0.9f});

    e1.insert<Test>(Test{5});
    e2.insert<Test>(Test{6});
    e3.insert<Test>(Test{7});
    e5.insert<Test>(Test{8});

    e1.insert<Test2>(Test2{1.9f});
    e2.insert<Test2>(Test2{1.0f});

    e4.insert<Test2>(Test2{1.7f});

    e4.insert<Test1>(Test1{9});

    component::logging::log(LEVEL_DEBUG, "Looping through Foo:");
    manager.query<Foo>().each([] (component::Entity entity, Foo& foo) {
        component::logging::log(LEVEL_DEBUG, "{} Foo: a={}", entity.id().value(), foo.a);
    });

    component::logging::log(LEVEL_DEBUG, "Looping through Bar:");
    manager.query<Bar>().each([] (component::Entity entity, Bar& bar) {
        component::logging::log(LEVEL_DEBUG, "{} Bar: a={}, b={}", entity.id().value(), bar.a, bar.b);
    });

    component::logging::log(LEVEL_DEBUG, "Looping through Bar2:");
    manager.query<Bar2>().each([] (component::Entity entity, Bar2& bar2) {
        component::logging::log(LEVEL_DEBUG, "{} Bar2: a={}, s=\"{}\"", entity.id().value(), bar2.a, bar2.s);
    });

    component::logging::log(LEVEL_DEBUG, "Looping through Baz:");
    manager.query<Baz>().each([] (component::Entity entity, Baz& baz) {
        component::logging::log(LEVEL_DEBUG, "{} Baz: a={}, b={}, c=\'{}\'", entity.id().value(), baz.a, baz.b, baz.c);
    });

    component::logging::log(LEVEL_DEBUG, "Looping through Test:");
    manager.query<Test>().each([] (component::Entity entity, Test& test) {
        component::logging::log(LEVEL_DEBUG, "{} Test: d={}", entity.id().value(), test.d);
    });

    component::logging::log(LEVEL_DEBUG, "Looping through TestBase:");
    manager.query<TestBase>().each([] (component::Entity entity, TestBase& test) {
        component::logging::log(LEVEL_DEBUG, "{} TestBase: getVal()={}", entity.id().value(), test.getVal());
    });

    component::logging::log(LEVEL_DEBUG, "Looping through Test2:");
    manager.query<Test2>().each([] (component::Entity entity, Test2& test) {
        component::logging::log(LEVEL_DEBUG, "{} Test2: f={}", entity.id().value(), test.f);
    });

    e5.erase<Test>();

    component::logging::log(LEVEL_DEBUG, "Looping through Test2 after erase:");
    manager.query<Test2>().each([] (component::Entity entity, Test2& test) {
        component::logging::log(LEVEL_DEBUG, "{} Test2: f={}", entity.id().value(), test.f);
    });

    e1.set(Foo{19});

    component::logging::log(LEVEL_DEBUG, "Looping through Foo, Test:");
    manager.query<Foo, Test>().each([] (component::Entity entity, Foo& foo, Test& test) {
        component::logging::log(LEVEL_DEBUG, "{} Foo: a={}, Test: d={}", entity.id().value(), foo.a, test.d);
    });

    component::logging::log(LEVEL_DEBUG, "Looping through Bar, Test:");
    manager.query<Baz, Test>().each([] (component::Entity entity, Bar& baz, Test& test) {
        component::logging::log(LEVEL_DEBUG, "{} Baz: a={}, b={}, Test: d={}", entity.id().value(), baz.a, baz.b, test.d);
    });

    e5.set(Test{8});

    component::logging::log(LEVEL_DEBUG, "Looping through Test2 after add back:");
    manager.query<Test2>().each([] (component::Entity entity, Test2& test) {
        component::logging::log(LEVEL_DEBUG, "{} Test2: f={}", entity.id().value(), test.f);
    });

    component::logging::log(LEVEL_DEBUG, "Looping through Foo pairs:");
    manager.query<Foo>().pairs([] (component::QueryBundle<Foo> e1, component::QueryBundle<Foo> e2) {
        //auto& [info1, foo1] = e1;

        //auto& [info2, foo2] = e2;

        component::logging::log(LEVEL_DEBUG, "{} Foo: a={}; {} Foo: a={}", e1.entity().id().value(), e1.get<Foo>().a, e2.entity().id().value(), e2.get<Foo>().a);
    });

    component::logging::log(LEVEL_DEBUG, "Looping through Test pairs:");
    manager.query<Test>().pairs([] (component::QueryBundle<Test> e1, component::QueryBundle<Test> e2) {
        //auto& [info1, test1] = e1;
        //auto& [info2, test2] = e2;

        component::logging::log(LEVEL_DEBUG, "{} Test: d={}; {} Test: d={}", e1.entity().id().value(), e1.get<Test>().d, e2.entity().id().value(), e2.get<Test>().d);
    });

    component::logging::log(LEVEL_DEBUG, "Looping through root entities:");
    for (auto i : manager.root()) {
        component::logging::log(LEVEL_DEBUG, "{}", i.id().value());
    }

    component::logging::log(LEVEL_DEBUG, "Looping through children of entities:");
    for (auto i : manager) {
        component::logging::log(LEVEL_DEBUG, "Children of {}:", i.id().value());
        for (auto j : i.children()) {
            component::logging::log(LEVEL_DEBUG, "{}", j.id().value());
        }
    }

    //e4.reparent(e5.id());
    e5.addChild(e4);

    component::logging::log(LEVEL_DEBUG, "Looping through root entities after reparent:");
    for (auto i : manager.root()) {
        component::logging::log(LEVEL_DEBUG, "{}", i.id().value());
    }

    component::logging::log(LEVEL_DEBUG, "Looping through children of entities after reparent:");
    for (auto i : manager) {
        component::logging::log(LEVEL_DEBUG, "Children of {}:", i.id().value());
        for (auto j : i.children()) {
            component::logging::log(LEVEL_DEBUG, "{}", j.id().value());
        }
    }


    component::logging::log(LEVEL_DEBUG, "Testing hierarchy insertion:", e1.id().value());
    e1.insert<Bar>(Bar{9, 9.9f});
    e4.insert<Foo>(Foo{10});

    component::logging::log(LEVEL_DEBUG, "Looping through Foo:");
    manager.query<Foo>().each([] (component::Entity entity, Foo& foo) {
        component::logging::log(LEVEL_DEBUG, "{} Foo: a={}", entity.id().value(), foo.a);
    });

    component::logging::log(LEVEL_DEBUG, "Looping through Bar:");
    manager.query<Bar>().each([] (component::Entity entity, Bar& bar) {
        component::logging::log(LEVEL_DEBUG, "{} Bar: a={}, b={}", entity.id().value(), bar.a, bar.b);
    });

    e2.remove();

    component::logging::log(LEVEL_DEBUG, "Looping through root entities after remove:");
    for (auto i : manager.root()) {
        component::logging::log(LEVEL_DEBUG, "{}", i.id().value());
    }

    component::logging::log(LEVEL_DEBUG, "Looping through children of entities after remove:");
    for (auto i : manager) {
        component::logging::log(LEVEL_DEBUG, "Children of {}:", i.id().value());
        for (auto j : i.children()) {
            component::logging::log(LEVEL_DEBUG, "{}", j.id().value());
        }
    }
};

int main (int argv, char* argc[]) {
    phenyl::PhenylEngine engine;
    testCompManager();

    logger::log(LEVEL_DEBUG, "MAIN", "Started game!");
    engine.run<test::TestApp>();

    return EXIT_SUCCESS;
}
