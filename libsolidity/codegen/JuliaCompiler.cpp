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
