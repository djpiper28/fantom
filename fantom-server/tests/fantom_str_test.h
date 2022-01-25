#pragma once
#include <cppunit/extensions/HelperMacros.h>

class TestFantomStr : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(TestFantomStr);
    CPPUNIT_TEST(test_init);
    CPPUNIT_TEST_SUITE_END();
public:
    TestFantomStr(void);
    void test_init();
};

