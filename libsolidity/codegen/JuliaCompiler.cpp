/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Alex Beregszaszi
 * @date 2017
 * Component that translates Solidity code into JULIA.
 */

#include <libsolidity/codegen/JuliaCompiler.h>
#include <boost/algorithm/string/predicate.hpp>

using namespace std;
using namespace dev;
using namespace dev::solidity;

void JuliaCompiler::error(ASTNode const& _node, string const& _description)
{
	auto err = make_shared<Error>(Error::Type::JuliaCompilerError);
	*err <<
		errinfo_sourceLocation(_node.location()) <<
		errinfo_comment(_description);
	m_errors.push_back(err);
}

void JuliaCompiler::fatalError(ASTNode const& _node, string const& _description)
{
	error(_node, _description);
	BOOST_THROW_EXCEPTION(FatalError());
}

bool JuliaCompiler::visit(ContractDefinition const& _contract)
{
	if (!_contract.baseContracts().empty())
		error(*_contract.baseContracts().front(), "Inheritance not supported.");
	if (!_contract.definedStructs().empty())
		error(*_contract.definedStructs().front(), "User-defined types not supported.");
	if (!_contract.definedEnums().empty())
		error(*_contract.definedEnums().front(), "User-defined types not supported.");
	if (!_contract.events().empty())
		error(*_contract.events().front(), "Events not supported.");
	if (!_contract.functionModifiers().empty())
		error(*_contract.functionModifiers().front(), "Modifiers not supported.");

	ASTNode::listAccept(_contract.definedFunctions(), *this);

	return false;
}

bool JuliaCompiler::visit(FunctionDefinition const& _function)
{
	if (!_function.isImplemented())
	{
		error(_function, "Unimplemented functions not supported.");
		return false;
	}
	if (_function.name().empty())
	{
		error(_function, "Fallback functions not supported.");
		return false;
	}
	if (!_function.modifiers().empty())
	{
		error(_function, "Modifiers not supported.");
		return false;
	}
	if (!_function.parameters().empty())
	{
		error(_function, "Parameters not supported.");
		return false;
	}
	if (!_function.returnParameters().empty())
	{
		error(_function, "Return parameters not supported.");
		return false;
	}

	assembly::FunctionDefinition funDef;
	funDef.name = _function.name();
	funDef.location = _function.location();
	m_currentFunction = funDef;
	_function.body().accept(*this);
	return false;
}

void JuliaCompiler::endVisit(FunctionDefinition const&)
{
	// invalidate m_currentFunction
	m_body.statements.emplace_back(m_currentFunction);
}

bool JuliaCompiler::visit(Block const& _node)
{
	for (size_t i = 0; i < _node.statements().size(); ++i)
		_node.statements()[i]->accept(*this);
	return false;
}

bool JuliaCompiler::visit(Throw const& _throw)
{
	assembly::FunctionCall funCall;
	funCall.functionName.name = "revert";
	funCall.location = _throw.location();
	m_currentFunction.body.statements.emplace_back(funCall);
	return false;
}

bool JuliaCompiler::visit(InlineAssembly const& _inlineAssembly)
{
	m_currentFunction.body.statements.emplace_back(_inlineAssembly.operations());
	return false;
}
