//
// Created by Leonid  on 2019-03-25.
//

#ifndef oatpp_test_websocket_FullTest_hpp
#define oatpp_test_websocket_FullTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace test { namespace websocket {

class FullTest : public oatpp::test::UnitTest {
public:

  FullTest() : UnitTest("TEST[FullTest]") {}

  void onRun() override;

};

}}}


#endif // oatpp_test_websocket_FullAsyncTest_hpp
