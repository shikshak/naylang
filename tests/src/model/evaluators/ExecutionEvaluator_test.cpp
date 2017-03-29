//
// Copyright (c) 2017 by Borja Lorente.
// Distributed under the GPLv3 license.
//

#include "catch.h"
#include <model/evaluators/ExecutionEvaluator.h>
#include <model/ast/expressions/primitives/BooleanLiteral.h>
#include <model/ast/declarations/VariableDeclaration.h>
#include <model/execution/objects/GraceBoolean.h>
#include <model/execution/objects/GraceNumber.h>
#include <model/execution/objects/GraceString.h>
#include <model/execution/objects/GraceScope.h>
#include <model/execution/objects/GraceDoneDef.h>
#include <model/ast/expressions/ObjectConstructor.h>
#include <model/ast/expressions/primitives/NumberLiteral.h>
#include <model/ast/expressions/primitives/StringLiteral.h>

using namespace naylang;

TEST_CASE("Execution Evaluator", "[Evaluators]") {
    auto ret = make_node<Return>();
    auto truRef = make_node<VariableReference>("tru");
    auto falRef = make_node<VariableReference>("fal");
    auto xRef = make_node<VariableReference>("x");
    auto truDec = make_node<VariableDeclaration>("tru");
    auto falDec = make_node<VariableDeclaration>("fal");
    auto xDec = make_node<VariableDeclaration>("x");
    auto trueLiteral = make_node<BooleanLiteral>(true);
    auto falseLiteral = make_node<BooleanLiteral>(false);
    auto ctruConstDecl = make_node<ConstantDeclaration>("ctru", trueLiteral);

    SECTION("An execution evaluator has a partial GraceObject") {
        ExecutionEvaluator eval;
        REQUIRE(*make_obj<GraceDoneDef>() == *eval.partial());
    }

    SECTION("Particular nodes") {
        SECTION("Evaluating a BooleanLiteral expression places a GraceBoolean on the partial") {
            ExecutionEvaluator eval;
            BooleanLiteral tru(true);
            REQUIRE_NOTHROW(eval.evaluate(tru));
            REQUIRE(eval.partial()->asBoolean().value());
        }

        SECTION("Evaluating a NumberLiteral expression places a GraceNumber on the partial") {
            ExecutionEvaluator eval;
            NumberLiteral num(5.0);
            REQUIRE_NOTHROW(num.accept(eval));
            REQUIRE(eval.partial()->asNumber().value() == 5.0);
        }

        SECTION("Evaluating a StringLiteral expression places a GraceString on the partial") {
            ExecutionEvaluator eval;
            StringLiteral str("hello");
            REQUIRE_NOTHROW(str.accept(eval));
            REQUIRE(eval.partial()->asString().value() == "hello");
        }

        SECTION("Evaluating a Return node terminates the execution") {
            // It assumes that the last executed expression is in the partial
            ExecutionEvaluator eval;
            Return ret;
            REQUIRE_NOTHROW(eval.evaluate(ret));
        }

        SECTION("Evaluating a Block places a GraceBlock on the partial") {
            ExecutionEvaluator eval;
            auto block = make_node<Block>();
            block->addParameter(truDec);
            block->addParameter(falDec);
            block->accept(eval);
            REQUIRE(eval.partial()->isBlock());
            REQUIRE(eval.partial()->hasMethod("apply"));
        }

        SECTION("Evaluating a variable reference throws if it's not there, and places it in the partial if it's there") {
            ExecutionEvaluator eval;
            auto xRef = make_node<VariableReference>("x");
            auto myScope = make_obj<GraceScope>();
            myScope->setField("x", GraceTrue);
            REQUIRE_THROWS(xRef->accept(eval));
            eval.setScope(myScope);
            REQUIRE_NOTHROW(xRef->accept(eval));
            REQUIRE(*GraceTrue == *eval.partial());
        }

        SECTION("Evaluating a Constant Declaration places the field in the current scope") {
            ExecutionEvaluator eval;
            auto tru = make_node<BooleanLiteral>(true);
            auto xDecl = make_node<ConstantDeclaration>("x", tru);
            eval.evaluate(*xDecl);
            REQUIRE(eval.currentScope()->hasField("x"));
            REQUIRE(*GraceTrue == *eval.currentScope()->getField("x"));
        }

        SECTION("Evaluating an ObjectConstructor creates a UserObject with all the nodes evaluated inside it") {
            ExecutionEvaluator eval;
            auto tru = make_node<BooleanLiteral>(true);
            auto xDecl = make_node<ConstantDeclaration>("x", tru);
            std::vector<StatementPtr> list{xDecl, tru};
            ObjectConstructor obj(list);

            GraceObjectPtr oldScope = eval.currentScope();
            eval.evaluate(obj);
            REQUIRE(eval.currentScope() == oldScope);
            REQUIRE(eval.partial()->hasField("x"));
        }
    }

    SECTION("Environment") {
        SECTION("The evaluator has a GraceScope environment object") {
            ExecutionEvaluator eval;
            REQUIRE(eval.currentScope()->isScope());
        }

        SECTION("An Execution Evaluator can create a scope") {
            ExecutionEvaluator eval;
            GraceObjectPtr newEnv = eval.createNewScope();
            REQUIRE(newEnv == eval.currentScope());
        }

        SECTION("An Execution Evaluator can restore a scope") {
            ExecutionEvaluator eval;
            GraceObjectPtr oldEnv = eval.currentScope();
            GraceObjectPtr newEnv = eval.createNewScope();
            eval.restoreScope();
            REQUIRE(oldEnv == eval.currentScope());
        }

        SECTION("It can set a passed scope") {
            ExecutionEvaluator eval;
            auto oldEnv = make_obj<GraceScope>();
            oldEnv->setField("x", GraceTrue);
            auto newEnv = make_obj<GraceScope>();
            newEnv->setField("y", GraceFalse);
            eval.setScope(oldEnv);
            REQUIRE(eval.currentScope()->hasField("x"));
            eval.setScope(newEnv);
            REQUIRE(!eval.currentScope()->hasField("x"));
            REQUIRE(eval.currentScope()->hasField("y"));
        }
    }

    SECTION("Native methods") {
        SECTION("Evaluating {!true} places GraceFalse on the partial") {
            ExecutionEvaluator eval;
            auto prefNot = make_node<ExplicitRequestNode>("prefix!", trueLiteral);
            eval.evaluate(*prefNot);

            REQUIRE(*GraceFalse == *eval.partial());
        }

        SECTION("Evaluating {true && false} places GraceFalse on the partial") {
            ExecutionEvaluator eval;
            std::vector<ExpressionPtr> prefParams{falseLiteral};
            auto prefNot = make_node<ExplicitRequestNode>("&&(_)", trueLiteral, prefParams);
            eval.evaluate(*prefNot);

            REQUIRE(*GraceFalse == *eval.partial());
        }

        SECTION("Evaluating {x -> !x}.apply(true) places GraceFalse on the partial") {
            ExecutionEvaluator eval;

            auto notX = make_node<ExplicitRequestNode>("prefix!", xRef);

            auto notBlock = make_node<Block>();
            notBlock->addParameter(xDec);
            notBlock->addStatement(notX);
            notBlock->addStatement(ret);

            std::vector<ExpressionPtr> applyReqParams{trueLiteral};
            auto applyReq = make_node<ExplicitRequestNode>("apply", notBlock, applyReqParams);

            applyReq->accept(eval);
            REQUIRE(*GraceFalse == *eval.partial());
        }
    }

    SECTION("Non-native methods") {
        SECTION("Evaluating the user defined my&&(true, false) (logic AND) places GraceFalse on top of the stack") {
            ExecutionEvaluator eval;
            std::vector<ExpressionPtr> andParams{falRef};
            auto andReq = make_node<ExplicitRequestNode>("&&(_)", truRef, andParams);
            // self.my&&(other) == (self && other)
            std::vector<DeclarationPtr> myAndParams{truDec, falDec};
            std::vector<StatementPtr> myAndBody{andReq, ret};
            auto myAndMeth = make_node<MethodDeclaration>("my&&(_,_)", myAndParams, myAndBody);

            std::vector<ExpressionPtr> requestParams{trueLiteral, falseLiteral};
            auto myAndReq = make_node<ImplicitRequestNode>("my&&(_,_)", requestParams);
            myAndReq->bindTo(*myAndMeth);

            eval.evaluate(*myAndMeth);
            eval.evaluate(*myAndReq);
            REQUIRE(*GraceFalse == *eval.partial());
        }

        SECTION("Methods inside objects can be evaluated with an ImplicitRequest inside the object, or an ExplicitRequest outside") {
            /*
            def x = object {
                    method f(param) {
                        return param
                    }
                    f
            };
            x.f
            */
            ExecutionEvaluator eval;

            auto paramDecl = make_node<VariableDeclaration>("param");
            auto paramRef = make_node<VariableReference>("param");
            std::vector<DeclarationPtr> paramDeclarations{paramDecl};
            std::vector<StatementPtr> body{paramRef};
            auto fDeclaration = make_node<MethodDeclaration>("f(_)", paramDeclarations, body);
            std::vector<ExpressionPtr> fImplCallParams{trueLiteral};
            auto fImplCall = make_node<ImplicitRequestNode>("f(_)", fImplCallParams);
            std::vector<StatementPtr> xConstructorBody{fDeclaration, fImplCall};
            auto xObjectConstructor = make_node<ObjectConstructor>(xConstructorBody);
            auto xConstDecl = make_node<ConstantDeclaration>("x", xObjectConstructor);

            xConstDecl->accept(eval);

            auto xRef = make_node<VariableReference>("x");
            std::vector<ExpressionPtr> fExplCallParams{falseLiteral};
            auto fExplicitCall = make_node<ExplicitRequestNode>("f(_)", xRef, fExplCallParams);

            fExplicitCall->accept(eval);
            REQUIRE(*GraceFalse == *eval.partial());
        }
    }
}