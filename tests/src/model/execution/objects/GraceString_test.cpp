//
// Copyright (c) 2017 by Borja Lorente.
// Distributed under the GPLv3 license.
//

#include <model/execution/objects/GraceString.h>
#include <model/execution/objects/GraceBoolean.h>
#include "catch.h"

using namespace naylang;

TEST_CASE("Grace String", "[GraceObjects]") {
    GraceString string("hello");
}

TEST_CASE("Grace String Native Methods", "[GraceString]") {
    auto helloStr = make_obj<GraceString>("hello");
    auto otherHelloStr = make_obj<GraceString>("hello");
    auto worldStr = make_obj<GraceString>("world");

    SECTION("Relational") {
        GraceString::Equals eq;
        GraceString::NotEquals neq;
        GraceString::Greater great;
        GraceString::GreaterEq greatEq;
        GraceString::Less less;
        GraceString::LessEq lessEq;

        SECTION("==(_) returns GraceTrue iff all characters on the strings are the same") {
            MethodRequest eqHello("==(_)", {otherHelloStr});
            MethodRequest eqWorld("==(_)", {worldStr});
            REQUIRE(*GraceTrue == *eq.respond(*helloStr, eqHello));
            REQUIRE(*GraceFalse == *eq.respond(*helloStr, eqWorld));
        }

        SECTION("!=(_) returns GraceFalse iff all characters on the strings are the same") {
            MethodRequest neqHello("!=(_)", {otherHelloStr});
            MethodRequest neqWorld("!=(_)", {worldStr});
            REQUIRE(*GraceFalse == *neq.respond(*helloStr, neqHello));
            REQUIRE(*GraceTrue == *neq.respond(*helloStr, neqWorld));
        }

        SECTION(">(_) returns GraceTrue iff C++ string comparison returns > 0") {
            MethodRequest grHello(">(_)", {otherHelloStr});
            MethodRequest grWorld(">(_)", {worldStr});
            REQUIRE(*GraceFalse == *great.respond(*helloStr, grHello));
            REQUIRE(*GraceFalse == *great.respond(*helloStr, grWorld));
        }

        SECTION(">=(_) returns GraceTrue iff C++ string comparison returns >= 0") {
            MethodRequest greqHello(">=(_)", {otherHelloStr});
            MethodRequest greqWorld(">=(_)", {worldStr});
            REQUIRE(*GraceTrue == *greatEq.respond(*helloStr, greqHello));
            REQUIRE(*GraceFalse == *greatEq.respond(*helloStr, greqWorld));
        }

        SECTION("<(_) returns GraceTrue iff C++ string comparison returns < 0") {
            MethodRequest lsHello("<(_)", {otherHelloStr});
            MethodRequest lsWorld("<(_)", {worldStr});
            REQUIRE(*GraceFalse == *less.respond(*helloStr, lsHello));
            REQUIRE(*GraceTrue == *less.respond(*helloStr, lsWorld));
        }

        SECTION("<=(_) returns GraceTrue iff C++ string comparison returns <= 0") {
            MethodRequest lseqHello("<=(_)", {otherHelloStr});
            MethodRequest lseqWorld("<=(_)", {worldStr});
            REQUIRE(*GraceTrue == *lessEq.respond(*helloStr, lseqHello));
            REQUIRE(*GraceTrue == *lessEq.respond(*helloStr, lseqWorld));
        }
    }
}