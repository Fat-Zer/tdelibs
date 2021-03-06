package Ast;
use strict;

use vars qw/ $this $pack @endCodes /;

#-----------------------------------------------------------------------------
# This package is used  to create a simple Abstract Syntax tree. Each node
# in the AST is an associative array and supports two kinds of properties -
# scalars and lists of scalars.
# See SchemParser.pm for an example of usage.
#                                                               ... Sriram
#-----------------------------------------------------------------------------

# Constructor 
# e.g AST::New ("personnel")
# Stores the argument in a property called astNodeName whose sole purpose
# is to support Print()

sub New {
	my ($this) = {"astNodeName" => $_[0]};
	bless ($this);
	return $this;
}

# Add a property to this object
# $astNode->AddProp("className", "Employee");

sub AddProp {
	my ($this) = $_[0];
	$this->{$_[1]} = $_[2];
}

# Equivalent to AddProp, except the property name is associated
# with a list of values
# $classAstNode->AddProp("attrList", $attrAstNode);

sub AddPropList {
	my ($this) = $_[0];
	if (! exists $this->{$_[1]}) {
		$this->{$_[1]} = [];
	}
	push (@{$this->{$_[1]}}, $_[2]);
}

# Returns a list of all the property names of this object
sub GetProps {
	my ($this) = $_[0];
	return keys %{$this};
}

1;
