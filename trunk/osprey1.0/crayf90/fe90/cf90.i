$set 1
0001 Message number %d, exceeds allowed range.
0002 Message level %d, is out of range.
0003 Message system failed trying to issue message %d
0004 The MODULE PROCEDURE statement is only allowed in a generic interface block.
0005 The %s statement is out of order.
0006 A keyword or identifier must follow the label or construct name.
0007 A construct name is not allowed on a %s statement.
0008 The %s statement must precede all component definitions in derived type definition "%s".
0009 No forward reference entries exist for label %s even though it is undefined.
0010 Dummy argument "%s" is already defined in this argument list.
0011 The line size option is ignored because free source form is in effect.
0012 %s underflow.
0013 The Fortran standard requires multiple entry point function results "%s" and "%s" to both be default intrinsic types.
0014 Insufficient memory is available for compilation to continue.
0015 The %s statement is not allowed in a block data program unit.
0016 The %s statement is not allowed in a main program unit.
0017 Cannot open "%s" file.  It is needed for debugging or tracing.
0018 The %s statement must not follow a MODULE PROCEDURE statement in a generic interface block.
0019 The %s statement is not allowed in a module.
0020 The use of DOUBLE COMPLEX is an extension to the Fortran standard.
0021 The type attributes for entry points "%s" and "%s" conflict.
0022 The type of function result "%s" must be a default intrinsic type if it does not match the type of other entry points.
0023 Label %s is not defined in this scoping unit.
0024 The character constant "%s", for the I/O control information list item %s, is not allowed.
0025 This statement is not allowed in a derived type definition.
0026 The %s statement is not allowed in an interface block; expected SUBROUTINE, FUNCTION, MODULE PROCEDURE or END statement.
0027 Function entry "%s" has result name "%s".  It must be used when declaring the result to be an array.
0028 A trailing ")" is missing in the program string.
0029 The maximum number of unnamed block data program units is 26 and has been exceeded.
0030 The Fortran standard does not allow more than one unnamed BLOCK DATA program unit.
0031 Arguments to a PROGRAM statement are an extension to the Fortran standard.
0032 The type specification "CHARACTER*(*)" is not allowed in an IMPLICIT statement.
0033 If the component is being typed as the derived type being defined, it must have the POINTER attribute.
0034 The name or attribute index is not valid.  It is less than 0 or greater than the table index.
0035 The character length of "%s" has exceeded the maximum length of %d.
0036 Function entry "%s" has result name "%s", which must be used when declaring the result to be a pointer.
0037 Input lines greater than 72 characters long are an extension to the Fortran standard.
0038 This character is not valid.  Expected keyword DO following failed parse of type spec DOUBLE.
0039 Namelist group name "%s" was referenced in an I/O statement.  It must not be specified in this NAMELIST statement.
0040 If this main program has no PROGRAM statement specified, then no name can be specified on the END PROGRAM statement.
0041 The %s statement must only be specified once for derived type definition "%s".
0042 Attribute entry %d has a variant problem with field %s.
0043 Only one %s statement is allowed in each %s construct.
0044 The entry point name of the host, "%s", must not be redefined in an interface body.
0045 Component "%s" is private type "%s", therefore derived type "%s" must also be private or have private components.
0046 The TASK COMMON statement is an extension to the Fortran standard.
0047 A '*' must only be used with the UNIT or FMT I/O control information list items.
0048 The DO variable must not be defined while it is active.
0049 Cannot open source file "%s".
0050 Attempt to read past end of file.
0051 A compiler directive must not be followed by a Fortran continuation line.
0052 More than %d continuation lines is an extension to the Fortran standard in %s source form.
0053 The first line of an include file must not be a continuation line.
0054 The source line following an INCLUDE line must not be a continuation line.
0055 The file line does not end with a newline.
0056 A continuation line must not contain a label.
0057 The INCLUDE file name length exceeds the maximum of %d characters.
0058 The INCLUDE file name is missing.
0059 There is a missing delimiter on an INCLUDE file name.
0060 The source following the INCLUDE file name is not a comment.
0061 The INCLUDE line must not contain a statement label.
0062 Operator "%s" is not intrinsic.  All operations in a DATA statement expression must be intrinsic.
0063 Cannot open INCLUDE file "%s".
0064 Recursive use of INCLUDE file "%s".
0065 A defined operator exceeds the maximum length of 31 characters.
0066 A defined operator is missing the "." delimiter.
0067 Identifier length exceeds the maximum of 31 characters.
0068 The label length exceeds the maximum of 5 digits.
0069 A label must have at least one nonzero digit.
0070 Each control item specifier must be specified only once.
0071 The Fortran standard requires continuation lines to contain text following the & symbol in free source form.
0072 Object "%s" is host associated to itself.  Attr index is %d.
0073 This control item specifier is not allowed for this I/O statement.
0074 Assembly language output (-eS or -S filename) disables binary output.  Assembly language output selected.
0075 Rounding/truncation conflict detected.  %s option selected.
0076 Inline option conflict detected.  Automatic mode selected.
0077 Command line has an unknown option "%c".
0078 -%c option has an illegal argument "%s".
0079 Options are not allowed after the input file name.
0080 Name must be specified for %s file when input is from stdin.
0081 Unexpected attribute entry "%s" (index = %d) has been found in the local scope during use processing.
0082 Debugging (-G) requires binary output (-eB).  Binary output is enabled.
0083 This token is missing the %c delimiter.
0084 This Hollerith constant should have %d characters, but it only contains %d characters.
0085 The number of characters in the Hollerith specifier must be greater than zero.
0086 The END statement of this %s must be followed by %s.
0087 In a real constant with a D or E following the decimal field, the D or E must be followed by an exponent.
0088 The kind parameter suffix is not valid on a double precision real constant form.
0089 The kind parameter on a literal constant must be an integer number or named constant.
0090 Boolean constants are an extension to the Fortran standard.
0091 This constant contains %d characters.  The number of allowed characters must be > 0 and <= %d.
0092 The value of this constant exceeds the range allowed on this machine type.
0093 Character '%c' is not valid in an octal constant.
0094 A Hollerith constant must be %d characters or less when the "R" form is specified.
0095 The real constant must contain digits in the whole and/or the fractional part of the constant.
0096 Hollerith constants are an extension to the Fortran standard.
0097 The constant is not assignment compatible with "%s".
0098 A dot operator consisting of only letters is expected.
0099 "-O %s" is specified, therefore "-O %s" must not be specified.
0100 This statement must begin with a label, a keyword, or an identifier.
0101 The length of the kind parameter exceeds the maximum length of 31.
0102 Only an assignment statement can be used on a WHERE statement.
0103 sgif90: SGI sgif90 Version %s (%s) %s
0104 sgif90: COMPILE TIME %f SECONDS
0105 sgif90: MAXIMUM FIELD LENGTH %d DECIMAL WORDS
0106 sgif90: %d SOURCE LINES
0107 sgif90: %d ERRORS, %d WARNINGS, %d OTHER MESSAGES, %d ANSI
0108 sgif90: CODE: %d WORDS, DATA: %d WORDS
0109 Blank common must not be declared in a TASK COMMON statement or directive.
0110 The SAVE attribute is specified more than once for common block "%s".
0111 The PARAMETER attribute is specified, therefore an initialization expression must be specified for "%s".
0112 The arithmetic IF statement is an obsolescent feature.
0113 IMPLICIT NONE is specified in the local scope, therefore an explicit type must be specified for data object "%s".
0114 A deferred-shape or assumed-shape array must not have an assumed-size specifier or an upper bound specified.
0115 An explicit-shape array must have an upper bound specified.
0116 The assumed-size specifier (*) must be the upper bound for the last dimension of an explicit-shape array declaration.
0117 The maximum rank for an array declaration is 7.
0118 Function "%s" was used in an expression.  The type is assumed to be %s.
0119 This is a deferred-shape dimension specifier.  Therefore a comma or right parenthesis is expected, but a "%s" was found.
0120 The mask expression of a WHERE statement or WHERE construct must be type logical.
0121 If the "= initialization" expression is specified, "::" must separate the type and attributes from the identifier list.
0122 The RESULT keyword is only permitted on ENTRY statements in function subprograms.
0123 "%s" is a derived type type-name.  It cannot be defined as a type-name more than once.
0124 Use of %s*%d is an extension to the Fortran standard.
0125 "%d" is not an allowed length for %s.
0126 Derived type "%s" is used, but it does not have any components defined for it.
0127 More than one part reference of this data reference has a rank greater than zero.
0128 Positional actual arguments in an actual argument list must precede any keywords.
0129 The kind type parameter must be a scalar integer constant.
0130 The kind type parameter value %s is not valid for type %s.
0131 Explicit-shape array "%s" has nonconstant bounds so it can only be declared in a function or subroutine program unit.
0132 Function entry "%s" has result name "%s", which must be used when declaring the result to be a target.
0133 The Fortran standard requires a SAVE statement with no saved-entity-list to be the only SAVE in the scoping unit.
0134 SGI pointers are an extension to the Fortran standard.
0135 A FORMAT statement is required to have a statement label.
0136 The variant active in the attribute entry passed to %s is not valid.
0137 An EQUIVALENCE list needs at least two items in the list.
0138 Table "%s" does not line up correctly on a word boundary for this machine.
0139 This I/O control list specifier requires a keyword.
0140 The derived type being defined is sequenced so any components which are derived types must also be sequenced.
0141 The beginning token of this statement has led to a bad choice of statement type.
0142 Label variable "%s" must be scalar and of type default integer.
0143 Character object "%s" has nonconstant bounds.  It can only be declared in a function or subroutine program unit.
0144 Transfer of control to the nonexecutable statement at line %d is not allowed.
0145 Transfer of control to the %s statement at line %d is not allowed.
0146 Statement label "%s" was previously defined at line %d.
0147 Transfer of control into the WHERE construct starting at line %d is not allowed.
0148 Transfer of control into the CASE block starting at line %d is not allowed.
0149 Transfer of control to an END IF statement from outside the IF construct is obsolescent.
0150 Transfer of control to an END DO statement from outside its block DO construct is not allowed.
0151 This use of derived type "%s" is not valid.
0152 The assignment statement of a WHERE statement or in a WHERE construct must not be a defined assignment.
0153 Transfer of control to an END SELECT statement from outside its CASE construct is not allowed.
0154 Transfer of control into the DO loop starting at line %d is not safe.
0155 Transfer of control into the DO loop starting at line %d is nonstandard.
0156 Transfer of control into the %s block starting at line %d is not safe.
0157 Transfer of control into the %s block starting at line %d is nonstandard.
0158 No name can be specified on the END BLOCK DATA statement for an unnamed block data program unit.
0159 More than one CASE DEFAULT statement was found for this CASE construct.
0160 There is a problem with the block stack at block stack index %d.
0161 A character literal used as a Hollerith constant is an extension to the Fortran standard.
0162 The %s statement is not allowed in an ELSE IF block of an IF construct.
0163 The %s statement is not allowed in an ELSE block of an IF construct.
0164 The %s statement is not allowed in an internal procedure.
0165 The %s statement is not allowed in a module procedure.
0166 Extraneous data follows a format specification.
0167 Field width is too small in this usage.
0168 Empty parentheses in a FORMAT are an extension to the Fortran standard and are ignored.
0169 A comma between these format fields is required by the Fortran standard.
0170 A comma preceding a right parenthesis is an extension to the Fortran standard.
0171 Construct name "%s" was previously declared at line %d.
0172 The assign variable name must be an unqualified name.
0173 The compiler expected a NAMELIST group name following the NML specifier.
0174 A WRITE statement with no unit I/O specifier is an extension to the Fortran standard.
0175 The IMPLICIT range "%c-%c" is not in alphabetical order.
0176 IMPLICIT NONE is specified in this scope, therefore no other IMPLICIT statements are allowed.
0177 The following letter(s) were specified in a previous IMPLICIT statement: "%s", so must not be specified again.
0178 Array class is Unknown_Array, but there is no declaration error for the dimension spec.
0179 Chose DEFAULT case on case statement in routine %s.  Should never get here.
0180 Identifier "%s" was found in table %s, but should not have been found.
0181 The mask expression in a WHERE statement or WHERE construct must be an array-valued expression.
0182 The use of a repetition count with the slash edit descriptor is an extension to the Fortran standard.
0183 The %s statement is not allowed in a CASE block of a CASE construct.
0184 Recursive function entry name "%s" must have a result name to be called recursively when declared an array.
0185 Function entry "%s" has a result name.  Use result name "%s" when typing the function result.
0186 Unexpected syntax:  Expecting "%s" to follow the END keyword, but found "%s".
0187 The "::" separator is required on a component definition statement, if the POINTER or DIMENSION attribute is specified.
0188 "%s" is already used as a component name for this derived type.
0189 Component "%s" has the POINTER attribute, therefore the DIMENSION attribute must be a deferred-shape dimension.
0190 Component "%s" does not have a POINTER attribute so its dimension must be an explicit-shape array with constant bounds.
0191 A character length for a component declaration must be a constant expression.
0192 The "* char-length" must only be specified for an object of type character.
0193 Unexpected value %d found in %s field for entry %d.
0194 The DO variable must be the name of a variable.
0195 In a masked array assignment, the mask expression and the variable being defined must be arrays of the same shape.
0196 Function %s was called with a token whose length exceeds %d characters.
0197 Unexpected syntax: "%s" was expected but found "%s".
0198 The "::" separator is illegal on the EXTERNAL and the INTRINSIC statements.
0199 The DO variable must be an unqualified name.
0200 The use of edit descriptor "%c" is an extension to the Fortran standard.
0201 An allocate object in an ALLOCATE statement must be either a pointer or an allocatable array.
0202 The status variable for this ALLOCATE/DEALLOCATE statement must be a scalar integer variable.
0203 The STAT= must follow all allocation items in an ALLOCATE statement.
0204 The number of subscripts is greater than the number of declared dimensions.
0205 If an array is specified in an ALLOCATE statement, it must have an allocate shape specification list.
0206 The exponent in a constant initialization expression must be type integer.
0207 Reset lex has attempted to reset source input to a line not in the source buffer.
0208 Using a SGI pointer as a DO variable is an extension to the Fortran standard.
0209 There is no actual argument to correspond to dummy argument number %d.
0210 "%s" has the %s attribute.  It must not be used in an initialization expression because it is not a constant.
0211 Continuation is not allowed for this statement.
0212 "%s" is typed as %s, therefore it must not have component references specified for it.
0213 "%s" is not a component of derived type "%s".
0214 The maximum number, %d, of fatal errors has been exceeded.
0215 Dummy argument "%s" of statement function "%s" must have a character length that is an integer constant expression.
0216 A character string must be less than %d characters when used as a dataset name or Boolean constant.
0217 The DO loop expression must be type integer, default real, or double precision real.
0218 A DO loop expression of type default real or double precision real is an obsolescent feature.
0219 The DO variable must be type integer, default real, or double precision real.
0220 A DO variable of type default real or double precision real is an obsolescent feature.
0221 Bound expressions other than integer are an extension to the Fortran standard.
0222 This DO statement expression must be scalar.
0223 The DO variable must be scalar.
0224 Semantic analysis of compiler generated DO loop expression failed.
0225 For [%s][%s], the error flag in table %s is TRUE (1), but the message number is 0.
0226 There are problems with the semantic tables.  See previous error messages.
0227 For [%s][%s], the error flag in table %s is FALSE (0), but the message number is %d.
0228 The mode specifier for a BUFFER IN or BUFFER OUT statement must be type integer.
0229 The unit specifier or file identifier is not a valid data type or form.
0230 The mode specifier for the BUFFER IN or BUFFER OUT statement must be scalar.
0231 The character string exceeds the maximum dataset name size of %d characters.
0232 IMPLICIT NONE is specified in the local scope, therefore an explicit type must be specified for function "%s".
0233 IMPLICIT NONE is specified in the host scope, therefore an explicit type must be specified for function "%s".
0234 The DO WHILE expression must be type logical.
0235 I/O list items of a derived type which have a pointer component are not allowed.
0236 NULL_IDX encountered when completing a compiler generated CONTINUE statement.
0237 Maximum table size has been exceeded for the "%s".
0238 "%s" is declared as a named constant.  The type is assumed to be %s.
0239 "%s" is initialized in a DATA statement.  The type is assumed to be %s.
0240 External function "%s" is referenced in a bounds specification expression.  It must have a nonpointer scalar result.
0241 The nonblock DO construct is an obsolescent feature.
0242 The termination statement of a nonblock DO construct must not be an unconditional GO TO statement.
0243 The termination statement of a nonblock DO construct must not be an assigned GO TO statement.
0244 The termination statement of a nonblock DO construct must not be a %s statement.
0245 The number of arguments in the call list is greater than the largest list expected.
0246 The termination statement of a nonblock DO construct must not be an outmoded IF statement.
0247 A subscript or substring expression in an EQUIVALENCE statement must be an integer constant expression.
0248 The DO statement label is already defined.
0249 Compiler internal error in fold_relationals.  Unexpected types of operands.
0250 Array syntax is not allowed in an EQUIVALENCE statement.
0251 Compiler internal error in fold_relationals.  Unexpected operator.
0252 The operands in this array syntax operation are not conformable.
0253 The left and right hand sides of this array syntax assignment must be conformable arrays.
0254 The values of the DO statement expressions prevent the loop from executing.
0255 The value of the increment expression must not be zero.
0256 Dummy argument "%s" has the POINTER attribute.  It requires an actual argument with the POINTER attribute.
0257 An illegal item "%d" is detected in field %s of the attribute table.
0258 This actual argument is an assumed-size array, therefore it must not be associated with an assumed-shape dummy argument.
0259 Recursive reference to "%s", a %s, is illegal in a bounds specification expression.
0260 Unexpected block stack order.
0261 The %s statement is not allowed in a function subprogram.
0262 The %s statement is not in the range of a matching DO construct.
0263 The semantics pass statement driver encountered an illegal or unknown statement type.
0264 The %s statement is not allowed following a CONTAINS statement.  Expecting FUNCTION, SUBROUTINE or END statement.
0265 Specifying the NAMELIST statement after the first executable statement is an extension to the Fortran standard.
0266 The use of edit descriptor -nX is an extension to the Fortran standard.
0267 The %s statement is not allowed in the THEN block of an IF construct.
0268 The %s statement is not allowed in a DO construct.
0269 The %s statement is not allowed following a SELECT CASE statement.
0270 The %s statement is not allowed in a WHERE construct.
0271 The %s statement is not allowed in an interface body.
0272 The %s statement is not allowed in a subroutine subprogram.
0273 The %s attribute is specified more than once for the component declaration.
0274 Fortran requires all intrinsic procedure actual arguments to be type integer or character for restricted expressions.
0275 The PUBLIC and/or PRIVATE attribute is specified more than once for derived type "%s".
0276 "%s" is a %s.  It must be a constant to be used in an initialization expression.
0277 "%s" is not valid as a keyword for this call to "%s".
0278 No semantics routine exists for statement type %s.
0279 The EXTERNAL attribute has been specified for "%s", the program unit being compiled.  This is nonstandard.
0280 Fortran requires all intrinsic procedure actual arguments to be type integer for specification expressions.
0281 The left hand side of an assignment statement must be a variable or a function result.
0282 Function "%s" is declared in an interface block, therefore it must not be typed assumed-length character.
0283 The name specified on the %s statement must match the %s name "%s".
0284 The %s construct has construct name "%s", therefore the %s statement must specify the same construct name.
0285 If the %s construct does not have a construct name, the %s statement must not have a construct name specified.
0286 A derived type type-name must not be the same as the name of the intrinsic type %s.
0287 The result of function name "%s" in the function subprogram is not defined.
0288 A termination statement is missing for this DO statement.
0289 This %s statement has no matching %s statement.
0290 Derived type "%s" must have at least one component declared for it.
0291 An END statement is missing for this %s statement.
0292 "%s" is specified as the module name on a USE statement, but the compiler cannot find it.
0293 This unnamed main program unit is missing an END statement.
0294 Equivalence-object "%s" must be a sequence derived type.
0295 Routine %s expected LA_CH input to be %s.
0296 PRINTMSG received a zero line number.  The message number to print is %d.  The column number is %d.
0297 IMPLICIT NONE is specified in the host scope, therefore an explicit type must be specified for data object "%s".
0298 Only one IMPLICIT NONE statement is allowed in a scope.
0299 An assignment can not be made to a function name when a separate function result variable has been declared.
0300 Operators "=, =>, and %%" must not be used as defined operators.
0301 Binary output must be requested for a module information file for module "%s" to be created.
0302 A rank %d operand and a rank %d operand are not conformant for "%s".
0303 Data type %s is not allowed with %s for the operation "%s".
0304 Missing left parenthesis in FORMAT specifier.
0305 Missing right parenthesis or unexpected end of FORMAT specifier.
0306 Integer constant expected where "%c" is present.
0307 A period is expected where "%c" is present.
0308 Expecting -nP, -nX, or +nP, but found "%c".
0309 No repeat count is allowed for the "%c" edit descriptor.
0310 The repeat count value must be greater than 0.
0311 Field width must not be zero.
0312 Maximum size for field width, decimal field width, exponent field width, or repetition count exceeded.
0313 Missing or zero length for the H edit descriptor.
0314 Unknown edit descriptor "%c" has been detected.
0315 Encountered end of data before completion of last edit descriptor processing.
0316 Hollerith, octal, hexadecimal and binary constants are not allowed as I/O list items.
0317 The use of ".%s." as a dot operator is an extension to the Fortran standard.
0318 The operator "%s" has not been declared as a user defined operator.
0319 A subscript must be a scalar integer expression.
0320 A vector subscript must be a rank 1 integer expression.
0321 The second subscript must not be omitted from a subscript triplet in the last dimension of an assumed-size array.
0322 This use of a function or derived type "%s" is not valid.
0323 The expressions in a substring range must be scalar integer expressions.
0324 Assignment of a rank %d expression to a rank %d variable is not allowed.
0325 The routine get_dummy_arg_type is called with a dummy argument that is not a data object.
0326 The left hand side of this assignment statement is a constant.
0327 The actual argument to IACHAR and ICHAR must have length equal to one character.
0328 Statement label %s is not a FORMAT statement label.
0329 Intrinsic function "%s" is not allowed in an initialization expression. 
0330 More than one actual argument has been specified for a dummy argument.
0331 There is no corresponding dummy argument for actual argument number %d.
0332 There is no actual argument to correspond to dummy argument "%s".
0333 An actual argument keyword is being used when an explicit interface is not known.
0334 The type of this actual argument does not match that of the dummy argument.
0335 Function "%s" is an illegal reference in a CALL statement.
0336 The kind of this actual argument does not match that of its associated dummy argument.
0337 The rank of this actual argument does not match that of its corresponding dummy argument.
0338 An alternate return dummy argument (*) requires a user label as its actual argument.
0339 Unexpected syntax:  A label, a left parenthesis, or a variable name is expected but found "%s".
0340 Variable "%s" does not appear in an ASSIGN statement in this scoping unit.
0341 The ASSIGN statement is an obsolescent feature.
0342 The assigned GO TO statement is an obsolescent feature.
0343 A RECURSIVE keyword must be declared for a subprogram so that the subprogram can be called recursively.
0344 This subprogram has no result name, but is being called recursively.  This is an extension to the Fortran standard.
0345 Label %s is not defined on either an executable statement or a FORMAT statement.
0346 The %s statement at line %d is not a valid branch target statement.
0347 Label %s is defined in the WHERE construct starting at line %d.
0348 Label %s is defined in the CASE block starting at line %d.
0349 Label "%s" does not appear in an ASSIGN statement in this scoping unit.
0350 "%s" is typed CHARACTER*(*), therefore it must be a dummy argument, a constant or a SGI pointee.
0351 "%s" is an assumed-shape array, therefore it must be a nonpointer dummy argument.
0352 "%s" has the %s attribute, therefore it must be a dummy argument.
0353 "%s" must have the POINTER or ALLOCATABLE attributes or be a dummy argument, because it is a deferred-shape array.
0354 Equivalence-object "%s" is of a derived type that has an ultimate component that has the POINTER attribute.
0355 "%s" is used in an expression or specified in a namelist group, therefore the type is assumed to be %s.
0356 Assignment of a %s expression to a %s variable is not allowed.
0357 The number of components in this derived type constructor does not match the declared number.
0358 This derived type constructor has a type mismatch error in component number %d.
0359 This structure constructor item which corresponds to a pointer component is not an allowable target.
0360 The rank of derived type constructor component number %d does not match the declared rank.
0361 Both actual arguments must be of type default real.
0362 This statement can not be reached.
0363 All of the actual arguments to this intrinsic call are not conformable to each other.
0364 The number of compiler generated labels exceeds the compiler limit of %d.
0365 This statement must not be the action-stmt of a logical IF statement.
0366 Unexpected statement type when parsing the target statement of a logical IF.
0367 Object "%s" is a module procedure or an internal function so it must not be typed as an assumed-length character.
0368 Module procedure "%s" must be defined in this scoping unit or associated from another scoping unit.
0369 This expression must be an integer scalar expression.
0370 Alternate return specifiers are not allowed in function subprograms.
0371 Alternate return specifiers are obsolescent.
0372 All dimension specifiers on a COMMON statement must be declared with constant bounds.
0373 Object "%s" is in a common block and is derived-type "%s".  This derived-type must be a sequence type.
0374 This numeric constant is out of range.
0375 The number of subscripts is smaller than the number of declared dimensions.
0376 An array reference with fewer than the declared number of dimensions is an extension to the Fortran standard.
0377 Unexpected syntax:  expected the keyword THEN, the beginning of an action-stmt, or a label. 
0378 This subprogram reference is illegal.
0379 Procedure "%s" must be EXTERNAL, a module procedure or declared in an interface block to be an actual argument.
0380 No specific match to the defined generic operation "%s" can be found.
0381 The PAUSE statement is obsolescent.
0382 Cannot open Compiler Information File "%s".
0383 Cannot write to the Compiler Information File.
0384 The length of the keyword at index %d in a keyword table is %d, which is greater than the maximum of 31.
0385 The character variable/expression or the integer constant following the STOP/PAUSE statement is nonstandard.
0386 The expression following a STOP/PAUSE must be a scalar character expression or an unsigned integer constant.
0387 The CONTAINS statement must be followed by a FUNCTION or SUBROUTINE statement.
0388 Assembly language listing (-rg) disables assembly language output (-eS or -S filename).  The listing is selected.
0389 No specific match can be found for the generic subprogram call "%s".
0390 Derived type "%s" is defined in the host scope and used in the local scope, therefore it must not be redefined.
0391 Type %s*%d will be mapped to %s*%d.
0392 Data type %s is not allowed for operator "%s".
0393 An array reference with parentheses must have at least one subscript.
0394 This Hollerith constant is longer than one word.
0395 Masking expressions are an extension to the Fortran standard.
0396 This generic interface has both subroutine and function specific interfaces.
0397 The generic interface "%s" cannot be called as a subroutine.
0398 The generic interface "%s" cannot be used as a function.
0399 Generic interface call "%s" resolved to "%s".
0400 The characters found in the label field are not valid.
0401 A stride specification in the array shape specification on the ALLOCATE statement is illegal.
0402 The number of extents specified for this allocate object does not match the declared rank.
0403 The upper or lower bound of a dimension in an allocate object must be a scalar integer expression.
0404 Only scalar objects can be allocated in an ALLOCATE statement.
0405 An allocate/deallocate object cannot be a substring reference.
0406 The indirect logical IF and the two-branch arithmetic IF statements are both nonstandard and outmoded.
0407 Labels must be in columns 1 through 5 in fixed source form.
0408 This pointer is to the right of a part-ref with nonzero rank.
0409 The conditional expression must be type integer, real, or double precision.
0410 The conditional expression must be scalar.
0411 A variable in an assignment statement must not be an assumed-size array.
0412 This whole array reference of an assumed-size array is not allowed.
0413 The STAT variable must not be allocated within the ALLOCATE statement in which it appears.
0414 The conditional expression must be type logical, integer, real, or double precision.
0415 A blank is required before this lexical token in free source form.
0416 The conditional expression must be type logical.
0417 The pointer-object in a pointer assignment statement must have the POINTER attribute.
0418 The variable in a pointer assignment statement must have the TARGET attribute or the POINTER attribute.
0419 The target in a pointer assignment statement must have the same type parameters as the pointer.
0420 The target in a pointer assignment statement must not be an array section with a vector subscript.
0421 An expression used in a pointer assignment statement must deliver a pointer result.
0422 Character '%c' is not valid in a binary constant.
0423 Character '%c' is not valid in a hexadecimal constant.
0424 Attribute %s appears more than once in this attribute list.
0425 Attributes %s and %s must not appear in the same attribute list.
0426 Each pointer-object in a NULLIFY statement must have the POINTER attribute.
0427 The STAT= variable must not be deallocated within the same DEALLOCATE statement.
0428 An allocate object must be either a pointer or an allocatable array in a DEALLOCATE statement.
0429 Only scalar objects can be deallocated in a DEALLOCATE statement.
0430 A deallocate object in a DEALLOCATE statement cannot be an array element or section.
0431 The target in a pointer assignment statement must have the same rank as the pointer.
0432 Pointer assignment of a %s target or pointer to a %s pointer is not allowed.
0433 Attempting to split an I/O statement without creating a begin and end I/O statement.
0434 Illegal association of a scalar actual argument to an assumed-shape dummy argument "%s".
0435 Illegal association of a scalar actual argument with array dummy argument "%s".
0436 Illegal association of an array-valued actual argument with a scalar dummy argument "%s".
0437 "%s" has the %s attribute, therefore it must not be used in a bounds specification expression.
0438 Namelist group "%s" has the PUBLIC attribute so namelist group object "%s" cannot have the PRIVATE attribute.
0439 The I/O control list for a %s statement must have a UNIT specifier.
0440 The I/O control list for an INQUIRE statement must have either a UNIT or FILE specifier.
0441 A type %s expression is not valid as an argument for the %s specifier of a %s statement.
0442 The INQUIRE statement must not have both the UNIT and FILE specifiers.
0443 The FMT and NML specifiers must not be specified in the same I/O control list.
0444 A namelist group name must not be present if an I/O list is present.
0445 The %s specifier cannot be used in a WRITE statement control list.
0446 The NML= control list specifier requires a namelist group name as an argument.
0447 The FMT= control list item must be a label, a character expression or an ASSIGN integer variable.
0448 This I/O control list item must be a label.
0449 A rank %d expression is not valid for the %s control list specifier.
0450 Procedure get_type_desc has been called with something that does not need a type descriptor.
0451 This reference to function %s without an actual argument list is illegal.
0452 This reference to subroutine %s is not in a CALL statement.
0453 This reference to main program %s is illegal.
0454 This reference to block data %s is illegal.
0455 This reference to module %s is illegal.
0456 Main program %s is an illegal reference in a CALL statement.
0457 Blockdata "%s" is an illegal reference in a CALL statement.
0458 Module %s is an illegal reference in a CALL statement.
0459 A type %s variable is not valid as an argument for the %s specifier of a %s statement.
0460 The %s control list specifier must have a scalar %s variable as an argument for this %s statement.
0461 The %s I/O control list specifier requires an argument with default kind parameter for the %s statement.
0462 If the FMT= specifier is an integer variable, it must be of default kind type.
0463 If REC= is present, an END= specifier must not appear in a %s statement.
0464 If REC= is present, the format must not be list-directed in a %s statement.
0465 If an EOR= specifier is present, an ADVANCE= specifier must also appear in a %s statement.
0466 If REC= is present, a namelist group name must not appear in a %s statement.
0467 A variable used as an internal file must not be an array section with a vector subscript.
0468 If an ADVANCE= specifier is present, the FMT= specifier must be present.
0469 If an ADVANCE= specifier is present, the FMT= specifier must not be list-directed.
0470 If the ADVANCE= specifier is present, the UNIT= specifier must not specify an internal file.
0471 If the UNIT= specifier refers to an internal file, the REC= specifier must not be present.
0472 If the UNIT= specifier refers to an internal file, a namelist group name must not be present.
0473 If the ADVANCE= specifier is present, the statement must be a sequential access I/O statement.
0474 The DO variable of an implied-DO loop must be integer, default real, or double precision real.
0475 Specific interface "%s" must be a subroutine, if it is inside a defined assignment interface block.
0476 This implied-DO control expression must be scalar.
0477 This implied-DO control expression must be an integer, default real, or double precision real expression. 
0478 An expression cannot be used as an input item in a %s statement.
0479 A constant cannot be used as an input item in a %s statement.
0480 Type %s*%d will be mapped to DOUBLE PRECISION.
0481 This implied-DO control variable is not valid.
0482 This implied-DO variable is not scalar.
0483 The IOLENGTH= specifier in an INQUIRE statement must be a scalar default integer variable.
0484 "%s" has an ultimate component that has the POINTER attribute, therefore it must not be in a namelist group.
0485 A character constant file name used as a UNIT= specifier is an extension of the Fortran standard.
0486 "%s" has been use associated from module "%s" and at least one more module.  It must not be referenced.
0487 The specific interfaces for "%s" and "%s" make the %s interface "%s" ambiguous.
0488 All bounds and kind selector expressions must be type integer, but this expression is type %s.
0489 The specific interface "%s" must have exactly two dummy arguments when inside a defined assignment interface block.
0490 Dummy argument "%s" must not have the OPTIONAL attribute within a defined operator or assignment interface block.
0491 Dummy argument "%s" must be specified with INTENT(%s) within a defined %s interface block.
0492 Specific interface "%s" must be a function within a defined operator interface block.
0493 The function result cannot be assumed-length character for "%s" because it is in a defined operator interface block.
0494 The specific interface "%s" must have exactly one dummy argument when inside a defined unary operator interface block.
0495 This specific interface "%s" conflicts with the intrinsic use of "%s".
0496 The specific interface "%s" must have exactly two dummy arguments when inside a defined binary operator interface block.
0497 The specific interface "%s" must have one or two dummy arguments when inside a defined operator interface block.
0498 Dummy argument "%s" is a dummy procedure.  This is not allowed within a defined %s interface.
0499 A defined operator must not be the same as a logical literal constant.
0500 Array "%s" is assumed-size, therefore it must be a dummy argument.
0501 "%s" is an assumed-size array of type character.  It must be a dummy argument or a pointee of a SGI character pointer.
0502 This actual argument is not a program unit.  Dummy argument "%s" is a dummy procedure.
0503 Argument mismatch:  Actual argument "%s" is a "%s".  Dummy argument "%s" is a "%s".
0504 Actual argument "%s" is a label but dummy argument "%s" is not an alternate return specifier.
0505 Continuation of character context in free source form requires an "&" as the first nonblank character of the next line.
0506 Function "%s" is recursive, therefore it must not be typed as assumed-length character.
0507 Function "%s" is array-valued and/or pointer-valued, therefore it must not be typed as assumed-length character.
0508 The parent-string of a substring reference must be of type character.
0509 "%s" is a %s, therefore it must not be used in a bounds specification expression.
0510 The use of ".%s." as a logical literal constant is an extension to the Fortran standard.
0511 "%s" is scalar, but it is being used as an array or function reference.
0512 "%s" is the name of a namelist group.  This is an illegal usage.
0513 Unexpected syntax:  A comma, identifier, WHILE, or EOS was expected but found "%c".
0514 malloc_check has found a problem with the malloc'd tables.  See previous message.
0515 Unexpected syntax:  A label, comma, identifier, WHILE, or EOS was expected but found "%c".
0516 For "%s", LN_NAME_IDX (%d) does not match AT_NAME_IDX(%d).
0517 This DO variable is already being used as the DO variable of an outer loop.
0518 "%s" is used as a subroutine, therefore it must not be used in a bounds specification expression.
0519 Dummy argument "%s" has the INTENT(OUT) attribute, therefore it must not be used in a bounds specification expression.
0520 Using external function "%s" in a bounds specification expression is nonstandard.  It should be PURE.
0521 Local variable "%s" must be a dummy argument or in common to be used in a bounds specification expression.
0522 It is an extension to the Fortran standard to allow character and noncharacter to be equivalenced to each other.
0523 The type of equivalence object "%s" is not identical to the type of the other equivalence objects.
0524 More than 99 continuation lines in fixed source form is not allowed.
0525 More than 99 continuation lines in free source form is not allowed.
0526 Common block "%s" has been extended past its start by equivalencing "%s".
0527 Object "%s" does not start on a word boundary.  This is caused by equivalence alignment.
0528 Object "%s" is equivalenced in two different places.
0529 Object "%s" has been previously used in an executable statement, therefore it must not be a dummy argument.
0530 An implied-DO variable must not be a variable subobject.
0531 Compiler directive %s must appear before the first executable statement.
0532 An input item must not be the implied-DO variable of any implied-DO loop that contains the input item.
0533 Reuse of "%s" as an implied-DO variable in a nested implied-DO is not allowed.
0534 "%s" is in blank common, therefore it must not be given the AUXILIARY attribute via a compiler directive statement.
0535 "%s" is typed as character, therefore it must not be given the AUXILIARY attribute via a compiler directive statement.
0536 "%s" is type "%s" which has a character or pointer component, therefore it must not be in auxiliary storage.
0537 "%s" is in TASK COMMON block "%s".  It must not be given the AUXILIARY attribute via a compiler directive statement.
0538 A negative base of type real cannot be raised to a real power.
0539 A continuation line is expected but this is not a continuation line.
0540 The value specified for DIM is invalid.
0541 Implementation is deferred for this Fortran statement, type, expression or constructor.
0542 The repeat factor must not be signed.
0543 Kind type parameter value %d will be mapped to %d.
0544 The termination statement of a nonblock DO construct must be an executable statement.
0545 Two operator nodes were marked IR_ARRAY_NODE in a reference tree.
0546 Nonarray used with subscript_opr in expr_semantics.
0547 "%s" is a named constant and is also a common block name.  This is an extension to the Fortran standard.
0548 "%s" has been used as a variable, therefore it must not be used as a derived-type name.
0549 A %s statement is not allowed within a parallel region.
0550 "%s" has the %s attribute, therefore it must not be declared with the %s attribute.
0551 "%s" is a %s, therefore it must not be declared with the %s attribute.
0552 "%s" has the %s attribute, therefore it must not be declared as a %s.
0553 "%s" is a %s, therefore it must not be declared as a %s.
0554 "%s" has the %s attribute.  It must not be given the %s attribute again.
0555 Cannot write to the temporary message file.
0556 Cannot open the temporary message file.
0557 "%s" has been used as a variable, therefore it must not be declared or used as a procedure.
0558 "%s" has been used as a variable, therefore it must not be declared as a %s.
0559 "%s" has been used as a variable, therefore it must not be declared with the %s attribute.
0560 "%s" is typed as variable length character, therefore it must not be declared with the %s attribute.
0561 "%s" is typed as variable length character, therefore it must not be declared as a %s.
0562 "%s" is an explicit-shape array with nonconstant bounds, therefore it must not be declared with the %s attribute.
0563 "%s" is an explicit-shape array with nonconstant bounds, therefore it must not be declared as a %s.
0564 "%s" is in a common block, therefore it must not be declared with the %s attribute.
0565 "%s" is in a common block, therefore it must not be declared as a %s.
0566 "%s" is defined in an explicit interface, therefore it must not be declared with the %s attribute.
0567 "%s" is defined in an explicit interface, therefore it must not be declared as a %s.
0568 "%s" is not a valid reference in a subroutine call.
0569 Label variable "%s" must be a variable, not a named constant.
0570 "%s" has the ALLOCATABLE attribute, therefore it must be specified as a deferred-shape array.
0571 Function result "%s" is a deferred-shape array, therefore it must have the POINTER attribute.
0572 "%s" has been used as a function, therefore it must not be declared with the %s attribute.
0573 "%s" has been used as a function, therefore it must not be declared as a %s.
0574 "%s" has been used as a subroutine, therefore it must not be declared with the %s attribute.
0575 "%s" has been used as a subroutine, therefore it must not be declared as a %s.
0576 "%s" has the %s attribute, therefore it must not be typed as variable length character.
0577 "%s" is a %s, therefore it must not be typed as variable length character.
0578 "%s" is in a common block, therefore it must not be typed as variable length character.
0579 "%s" is defined in an explicit interface.  All information about this procedure must be specified in the interface.
0580 The data type of object "%s" is %s and is incompatible with data type %s in this PARAMETER assignment.
0581 "%s" is a subroutine, therefore it must not be given an explicit type.
0582 "%s" has the %s attribute, therefore it must not be declared as an explicit-shape array with nonconstant bounds.
0583 "%s" is a %s, therefore it must not be declared as an explicit-shape array with nonconstant bounds.
0584 Machine characteristics system call "%s" failed.
0585 "%s" is in a common block, therefore it must not be declared as an explicit-shape array with nonconstant bounds.
0586 The -dp option does not affect REAL(KIND=16) or COMPLEX(KIND=16).  These will remain double precision.
0587 The initialization expression must be a constant to be used with PARAMETER assignment for object "%s".
0588 "%s" is a subroutine, therefore it must not be declared with the DIMENSION attribute.
0589 "%s" has the %s attribute, therefore it must not be in a common block.
0590 "%s" is a %s, therefore it must not be in a common block.
0591 "%s" is typed as variable length character, therefore it must not be in a common block.
0592 "%s" is an explicit-shape array with nonconstant bounds, therefore it must not be in a common block.
0593 "%s" already appears in common block "%s".  It must only be specified once in a common block.
0594 "%s" is a procedure name, therefore it must not be in a common block.
0595 The %s attribute on the type declaration statement is not allowed in a %s program unit.
0596 The %s attribute must only be specified if the derived type statement is in the specification part of a module.
0597 Compiler internal development command line option caused the compiler to abort on the first ANSI message.
0598 "%s" must not have the PUBLIC attribute, because it is typed as private type "%s".
0599 "%s" has the %s attribute, therefore it must not be in a namelist group.
0600 "%s" is a %s, therefore it must not be in a namelist group.
0601 "%s" is typed as variable length character, therefore it must not be in a namelist group.
0602 "%s" is an explicit-shape array with nonconstant bounds, therefore it must not be in a namelist group.
0603 "%s" is a procedure name, therefore it must not be in a namelist group.
0604 -%c %s is an unsupported compiler directive.
0605 "%s" must be a constant to be used in an initialization expression.
0606 Internal Error with I/O control list table.
0607 Object "%s" is in auxiliary storage.  It must not be host associated.
0608 "%s" has the %s attribute, therefore it must not be a procedure in an interface block.
0609 "%s" is a %s, therefore it must not be a function in an interface block.
0610 "%s" is typed as variable length character, therefore it must not be a procedure in an interface block.
0611 "%s" is an explicit-shape array with nonconstant bounds, therefore it must not be a procedure in an interface block.
0612 "%s" is in a common block, therefore it must not be a procedure or be used as a procedure.
0613 "%s" is defined in an explicit interface.  It must not be defined with more than one explicit interface.
0614 The storage size needed for "%s" exceeds %d words, the maximum storage size available.
0615 The storage size needed for this expression exceeds %d, the maximum storage size available.
0616 "%s" has been used as a function, therefore it cannot be declared in an explicit interface block.
0617 "%s" has been used as a subroutine, therefore it cannot be declared in an explicit interface block.
0618 Precompiled procedure "%s" must have the same double precision (-ep/-dp) option specified as this compilation.
0619 "%s" is a %s, therefore it must not be a subroutine in an interface block.
0620 Dummy argument "%s" is not an auxiliary array but this actual argument is.
0621 The kind parameter "%s" is not valid.  It must be a scalar integer constant.
0622 Dummy argument "%s" has the TARGET attribute.  It requires an actual argument with the TARGET or POINTER attribute.
0623 Default integer kind size for precompiled procedure "%s" must match the default integer kind size for this compilation.
0624 Bad left operand to ptr_assign_from_target.
0625 "%s" is a %s.  To be a SGI character pointee, it must be declared %s prior to the POINTER statement.
0626 Expected %s in %s.
0627 Dummy argument "%s" is an auxiliary array and the actual argument is not.
0628 "%s" has the %s attribute, therefore it must not be used as an external function name.
0629 "%s" is a %s, therefore it must not be used as an external function name.
0630 Non constant character in namelist_static_dv_whole_def.
0631 This compiler is not licensed on this hardware.  Please contact craysoft.
0632 "%s" has been used as a subroutine, therefore it must not be used as a function.
0633 "%s" has the %s attribute, therefore it must not be used as a subroutine name in a CALL statement.
0634 "%s" is a %s, therefore it must not be used as an external subroutine name in a CALL statement.
0635 "%s" is typed as character, therefore it must not be used as a subroutine.
0636 "%s" has the DIMENSION attribute, therefore it must not be used as a subroutine.
0637 Something other than list opnd in io_list_semantics.
0638 SGI character pointee "%s" must be an assumed-length character variable.
0639 "%s" is used as a function, therefore it must not be used as a subroutine.
0640 This actual argument must not be scalar.
0641 "%s" is a %s, therefore it must not be used in an expression.
0642 "%s" is already a %s, therefore it must not be declared a %s.
0643 "%s" has been used as a subroutine, therefore it must not be referenced in an expression.
0644 "%s" has the %s attribute, therefore it must not be used as a derived-type name.
0645 "%s" is a %s, therefore it must not be used as a derived-type name.
0646 "%s" is typed as character, therefore it must not be used as a derived-type name.
0647 "%s" has the DIMENSION attribute, therefore it must not be used as a derived-type name.
0648 "%s" is in a common block which makes it a variable, therefore it must not be used as a derived-type name.
0649 "%s" is a procedure, therefore it must not be used as a derived-type name.
0650 "%s" is a SGI pointee.  It must not be typed as a derived type.
0651 "%s" is typed as a derived type, therefore it must not be declared as a SGI pointee.
0652 "%s" has been used as a function, therefore it must not be used as a derived-type name.
0653 "%s" has been used as a subroutine, therefore it must not be used as a derived-type name.
0654 This actual argument does not have a legal rank for this intrinsic procedure.
0655 fnd_semantic_err is detecting error(s) that the calling routine, %s, is not.
0656 A PUBLIC/PRIVATE statement with no access list must only be specified once in the scoping unit of module "%s".
0657 Function result "%s" cannot be in a namelist group, because it is not the result of an active function.
0658 "%s" must be an integer constant or the DO variable of a containing implied-DO.
0659 All dummy arguments used in the length expression for object "%s" must be specified at the same entry point.
0660 All dummy arguments used in the bounds expressions for array "%s" must be specified at the same entry point.
0661 All dummy arguments used in specification expressions to declare "%s" must be specified at the same entry point.
0662 All dummy arguments used in expressions to declare "%s" must be specified at the same entry point as the object.
0663 "%s" belongs to common block "%s" that is in auxiliary storage so it must not be a namelist group object.
0664 Expression semantics missed detection of an error for a constant specification expression.
0665 Unexpected basic type from create_dv_type_code.
0666 "%s" has been used as a variable or a function in the character length, therefore it must not be the function name.
0667 The number of entities to be initialized exceeds the number of values.
0668 The number of values exceeds the number of entities to be initialized.
0669 The matching DO statement has a construct name, therefore this statement must be an END DO with the same construct name.
0670 Number of list items is greater than list count value.
0671 Procedure "%s" is already in interface block "%s".  It must not be specified multiple times.
0672 "%s" has been used as a function, therefore it must not be declared an explicit-shape array with nonconstant bounds.
0673 The dimension attributes for entry points "%s" and "%s" conflict.
0674 Entry points "%s" and "%s" must both have the POINTER attribute.
0675 An implied-DO variable must be the name of a variable of type integer.
0676 An identifier in a DATA value list must be the name of a previously declared named constant.
0677 If the repeat factor is an identifier, it must be the name of a scalar named constant.
0678 A DATA statement repeat factor must be type integer and be a scalar constant.
0679 The value of a DATA statement repeat factor must be equal to or greater than zero.
0680 More than one IF condition is specified for %s tasking directive.
0681 The specifier for number of characters in this %s statement must be of type integer.
0682 The specifier for number of characters in this %s statement must be in the range 1 to 152.
0683 The specifier for number of characters in this %s statement must be a scalar integer expression.
0684 Module procedure "%s" must have the PRIVATE attribute, because its function result is a private type.
0685 Module procedure "%s" must have the PRIVATE attribute, because dummy argument "%s" is a private type.
0686 Generic interface "%s" must be private, because specific interface "%s" is a function with a private type.
0687 Generic interface "%s" must be private, because dummy argument "%s" for specific interface "%s" is a private type.
0688 Common block "%s" was specified in a SAVE statement, but has not been specified in a COMMON statement.
0689 The operator enum in globals.h does not match the operator_str array in debug.h.
0690 Common block "%s" is specified in a TASK COMMON directive, therefore it must be specified in a COMMON statement.
0691 "%s" is of a derived type that has an ultimate component that is a pointer, therefore it must not be a named constant.
0692 Initializing a named common block member in a program unit other than a block data program unit is nonstandard.
0693 A blank common block member must not be initialized.
0694 This Hollerith constant is longer than one word.
0695 A SGI character pointer must not be initialized.
0696 The implied-DO variable has already been used as an implied-DO variable in an inner loop.
0697 SGI pointer "%s" and  Cray pointee "%s" both must be public or they both must be private.
0698 Partial initialization of a whole array is an extension to the Fortran standard.
0699 DATA target is a function reference or an array element reference to an undeclared array.
0700 No specific intrinsic exists for the intrinsic call "%s".
0701 "%s" is not an intrinsic routine.
0702 Type double complex is not supported with -ep.
0703 A continuation line can only follow a line continued with the "&" symbol.
0704 Unexpected FLD value.
0705 "%s" is not a constant therefore it must not appear in an initialization expression.
0706 This function name must not appear in a DATA statement expression.
0707 Module procedure "%s" is not a module procedure in a parent of this scoping unit.
0708 Possible recursive reference to module procedure "%s".  RECURSIVE must be specified for a recursive reference.
0709 A DATA implied-DO target must be an array element or scalar structure component reference.
0710 The -dp option does not change type %s*%d.  It will remain double precision.
0711 The type statement for generic intrinsic function %s is ignored.
0712 Generic interface "%s" must not also be the name of a module %s, unless it is a specific name in generic interface "%s".
0713 Generic interface "%s" is also an external or module procedure.  It must be a specific name in generic interface "%s".
0714 "%s" is a common block name and is declared as an intrinsic procedure.  This is nonstandard.
0715 Binary output (-eB or -b filename) disables assembly language output (-eS or -S filename).  Binary file selected.
0716 "%s" is an external procedure or a program unit and is also a common block name.  This is nonstandard.
0717 The -Ta option implies -Ca.
0718 The line number passed to the routine set_format_start_idx is not a valid line number.
0719 Integer overflow resulted from an integer conversion or arithmetic operation.
0720 A constant expression exceeds the valid range.
0721 A divisor of zero was detected in an expression.
0722 This use of function "%s" is not valid.
0723 This use of a generic interface or internal function "%s" is not valid.
0724 Unknown statement.  Expected assignment statement but found "%s" instead of "=" or "=>".
0725 Module "%s" must be targeted for the same operating system as this compilation.
0726 There is a problem with the module information file for module "%s".
0727 The BOUNDARY argument must have the same type and type parameters as ARRAY.
0728 At least one actual argument to this intrinsic must be present.
0729 "%s" is not type integer, therefore initializing it with a BOZ literal constant is nonstandard.
0730 A dummy argument with INTENT(OUT) or INTENT(INOUT) is associated with an actual argument that cannot be defined.
0731 "%s" is the name of a dummy argument to program unit "%s", therefore it must not be use associated from module "%s".
0732 "%s" is not in module "%s".
0733 Initializing multiple array elements with a single Hollerith or character constant is nonstandard.
0734 Internal error in create_loop_stmts. 
0735 Using a single END DO as a termination statement for multiple DO loops is an extension to the Fortran standard.
0736 "%s" is the name of this program unit, therefore it must not be use associated from module "%s".
0737 Internal compiler error encountered in process_deferred_io_list.
0738 Dummy argument X is type complex, thus dummy argument Y must not be present.
0739 No arguments are allowed to the "%s" intrinsic function - arguments ignored.
0740 IMPLICIT NONE is specified in the local scope, therefore an explicit type must be specified for statement function "%s".
0741 IMPLICIT NONE is specified in the local scope.  Statement function dummy argument "%s" must have an explicit type.
0742 IMPLICIT NONE is specified in the host scope, therefore an explicit type must be specified for statement function "%s".
0743 IMPLICIT NONE is specified in the host scope.  Statement function dummy argument "%s" must have an explicit type.
0744 The -%c command line option has an unsupported option "%s".
0745 The case value is not the same type as the case expression in the SELECT CASE statement.
0746 The case value has the same value as a case value on line %d.
0747 The case-value is already contained in a case-value range on line %d.
0748 The case-value range contains a value that was already specified by a case-value on line %d.
0749 The case-value range overlaps a case-value range on line %d.
0750 Actual argument number %d to statement function "%s" is not scalar.
0751 Actual argument has type "%s".  Associated dummy argument "%s" has type "%s".
0752 Actual argument number %d to statement function "%s" has a different kind type than the dummy argument.
0753 Statement function "%s" must not call itself recursively.
0754 Statement function "%s" is called with an incorrect number of arguments.
0755 The expression of statement function "%s" must be a scalar expression.
0756 The type of the expression is not assignment compatible with the result of statement function "%s".
0757 Operator "%s" is not intrinsic.  All operations in a statement function expression must be intrinsic.
0758 The value to the left of the colon is greater than the value to the right of the colon. 
0759 "%s" has the %s attribute, so it is a procedure.  Statement function actual arguments must be scalar data objects.
0760 "%s" is a %s.  Statement function actual arguments must be scalar data objects.
0761 "%s" is a procedure name, therefore it must not be used as an actual argument to a statement function.
0762 Internal error in io_ctl_list_semantics.
0763 Namelist descriptor was not created for %s.
0764 The SELECT CASE case-expr is type logical therefore a colon must not appear.
0765 The SELECT CASE case-expr must be scalar.
0766 The case-value must be a scalar expression.
0767 The SELECT CASE case-expr must be type integer, character, or logical.
0768 The case-value must be type integer, character, or logical.
0769 Unexpected syntax:  Expecting "%s" or EOS to follow the END keyword, but found "%c". 
0770 The kind-selector expression must be a scalar integer initialization expression.
0771 Use of a BOZ literal constant outside of a DATA statement is an extension to the Fortran standard.
0772 The rank of this actual argument must match that of assumed-shape dummy argument "%s".
0773 The rank of this actual argument does not match that of the dummy argument "%s".
0774 Improper intrinsic argument type or inconsistent types.
0775 An extent tmp has unexpected IR, during bound expansion at a call site.  The tmp is "%s" (%d).
0776 "cif_stmt_type_rec" was passed a statement type it did not expect.
0777 The actual argument to the PRESENT intrinsic must be an optional dummy argument.
0778 This format specifier is an extension to the Fortran standard.
0779 LOC function argument must be a variable, array, or array element.
0780 Statement label "%s" was previously defined in this derived type definition.
0781 The substring start or end value exceeds the defined range.
0782 List entry %d has a variant problem with field %s.
0783 The TARGET argument must have the TARGET attribute or the POINTER attribute.
0784 The first argument to this intrinsic must have the POINTER attribute.
0785 The autotasking directive "%s" must only be used within a parallel region bounded by PARALLEL/ENDPARALLEL directives.
0786 An actual argument must be definable when associated with a dummy argument that has INTENT(OUT) or INTENT(INOUT).
0787 Intrinsic "%s" is an extension to the Fortran standard.
0788 If -G0 (full debug) is specified, all optimizations are turned off.
0789 The case-value-range is missing both its case-values.
0790 Unknown compiler directive or syntax error.
0791 "%s" is not a module, therefore it must not be specified on the USE statement as the module-name.
0792 "%s" is the name of the module being compiled.  It must not be specified on a USE statement.
0793 Module "%s" has no public entities declared in the module, therefore names specified in the only-list cannot be found.
0794 !DIR$ INTEGER= must either precede a program unit or must precede any statements within the program unit.
0795 Compiler directive %s is only allowed within a program unit.
0796 The specified SAFEVL= value on the IVDEP directive must be a scalar integer constant in the range 1 to 1024.
0797 The command line has an unsupported option "%c".
0798 Unexpected parameter or work distribution for the DO ALL directive.
0799 The use of "%s" as a continuation character is an extension to the Fortran standard.
0800 Only one work distribution must be specified on a DO ALL directive.
0801 Unsupported compiler directive.
0802 Variable subobjects are not allowed as arguments for this parameter of a tasking compiler directive.
0803 The expression for the tasking directive parameter IF must be a scalar logical expression.
0804 "%s" is not a variable and will be ignored in the %s list of this %s directive.
0805 Object "%s" is declared both SHARED and PRIVATE.
0806 This argument on a tasking directive must be a scalar integer expression.
0807 The SHAPE argument must be a constant size rank one array with 7 or less elements.
0808 Unexpected work distribution for the DO PARALLEL directive.
0809 Unexpected parameter for the PARALLEL compiler directive.
0810 "%s" has a bad linear type.
0811 The case-value expression must be an initialization expression.
0812 This ENDCASE autotasking directive has no matching CASE directive.
0813 This END DO directive has no matching DO PARALLEL directive.
0814 This DO ALL autotasking directive is already within a parallel region.
0815 Unexpected nesting or overlap of GUARD regions.
0816 Matching GUARD/ENDGUARD directives must both have the same flag argument or have no argument.
0817 This ENDGUARD directive has no matching GUARD directive.
0818 The PARALLEL directive must not be used within another parallel region.
0819 This END PARALLEL directive has no matching PARALLEL directive.
0820 An END CASE directive was expected before the end of the previous parallel region.
0821 A call to an internal procedure is illegal within a parallel region.
0822 An "%s" autotasking directive was expected.
0823 Two different common blocks, "%s" and "%s", are equivalenced together.   
0824 The pdgcs symbol table index for attribute index %d ("%s") is 0.  This must be a valid index.
0825 The object being initialized is not a member of any common block declared in this block data program unit.
0826 Two different objects, "%s" and "%s", in the same common block are equivalenced together.
0827 "%s" is used in an expression, before it is typed via an implicit or type statement, so the type is assumed to be %s.
0828 Unsupported operator in folder_driver.
0829 All ac-value expressions in an array constructor must have the same type and type parameters.
0830 There must be only one or zero arguments present when calling this intrinsic.
0831 The value of this subscript is less than the declared lower bound.
0832 The compiler has detected a local name table following the scope for "%s".
0833 The argument to this intrinsic must be an allocatable array.
0834 The shape of the initialization expression value does not match that of named constant "%s".
0835 The initialization of scalar named constant "%s" with an array-valued expression is not valid.
0836 Compiler tmp "%s" does not have a storage block assigned to it.
0837 An implied-DO variable that is not scalar is an extension to the Fortran standard.
0838 Array constructor values of type character must all have the same length.
0839 The assumed-length character bounds entry from decl_semantics is not valid.
0840 "%s" is substringed but not declared type character, or the object is subscripted but not dimensioned.
0841 The start value of the section subscript triplet is less than the declared lower bound.
0842 The initialization expression used on a type declaration statement must be a constant expression.
0843 The data type of object "%s" is %s and is incompatable with data type %s in this initialization assignment.
0844 The initialization of scalar variable "%s" with an array-valued expression is not valid.
0845 The shape of the initialization expression does not match that of variable "%s".
0846 Storage block "%s" should be in the storage block table for the local scope, but is not.
0847 "%s" does not have a storage block.
0848 Dummy argument "%s" is longer than the actual argument.
0849 The start value of the section subscript triplet is greater than the declared upper bound.
0850 "%s" is host associated and on the stack, but it is not in a host associated stack storage block.
0851 A task common block member must not be data initialized.
0852 Host associated object "%s" must be referenced or defined by a contained procedure.
0853 The target must have the same character length as the pointer in a pointer assignment statement.
0854 The compiler cannot open file "%s", which may contain information about the used module "%s".
0855 The compiler has detected errors in module "%s".  No module information file will be created for this module.
0856 DO loop iteration count exceeds the compiler limit of %d iterations.
0857 DO loop iteration count calculation failed.
0858 An object with the SAVE attribute must not be equivalenced to an object in a common block.
0859 A variable subobject is not valid on a SUPPRESS directive.
0860 The intrinsic "%s" cannot be passed as an actual argument.
0861 Bad IR in %s.
0862 The storage order of object "%s" has been altered through equivalencing.
0863 "%s" has been referenced in the function character length expression, therefore it must not be declared as a "%s".
0864 "%s" has been referenced in the function character length expression, so it must not be a subroutine.
0865 Inline option conflict detected.  Non-automatic mode selected.
0866 The function character length expression references "%s", therefore it must not be used as a derived-type name.
0867 Module "%s" has no public objects declared in the module, therefore nothing can be use associated from the module.
0868 "%s" is used in a constant expression, therefore it must be a constant.
0869 "%s" has been typed, but the intrinsic procedure with the same name is a subroutine and subroutines cannot be typed.
0870 The intrinsic call "%s" is not valid in a specification expression.
0871 Field "%s" for statement header %d indexes to itself.
0872 The implied-do-object-list is missing for this implied-DO.
0873 The length tmp "%s" (%d) can only have one statement associated with it.  This one has multiple statement headers.
0874 The argument to the compiler directive ID must be a character literal.
0875 This actual argument must not be an optional dummy argument.
0876 "%s" has the AUXILIARY attribute and is in the specification part of a module, therefore it must be in a common block.
0877 The -u options are ignored because this is not a DEBUG version of the compiler.
0878 A module named "%s" has already been directly or indirectly use associated into this scope.
0879 The bloc or eloc variables must not be of derived type in a BUFFER I/O statement.
0880 Operator "%s" is not intrinsic.  All operations in a specification expression must be intrinsic.
0881 The value of DIM must be:  1 <= DIM <= (rank of MASK).
0882 Derived type "%s" has private components, which means component name "%s" must not be referenced.
0883 Derived type "%s" has private components, therefore a structure constructor must not be defined for this type.
0884 Verify_interface has found a bad dummy argument "%s" in the dummy argument list for procedure "%s".
0885 Inlining is a deferred implementation.
0886 Implementation of the -G %d debugging level is deferred.
0887 The %s optimization level should be %d since the debugging level is %d.
0888 The BOUNDARY argument must be present if the ARRAY argument is of derived type.
0889 The upper bound or extent of an assumed-size array must not be requested.
0890 "%s" is a dummy argument with INTENT(IN), therefore it must not be defined.
0891 Attribute %d (%s) has no TYPE.  It should because it is a Data_Obj.
0892 The H edit descriptor is an obsolescent feature.
0893 Fields %s and %s for %s entry %d must both be set to a legal value.  One field is set and the other is clear.
0894 Module "%s" has compile errors, therefore declarations obtained from the module via the USE statement may be incomplete.
0895 Intrinsic operator encountered in constant constructor that has no folder.
0896 The data item length must be equal for bloc and eloc in a BUFFER I/O statement.
0897 This call to VFUNCTION "%s" is not allowed inside a WHERE block or on a WHERE statement.
0898 "%s" is declared as a scalar variable in a module, therefore it must not be used as an external function name.
0899 Use of the tab character is an extension of the Fortran standard.
0900 Use of the "@" character in identifiers is an extension to the Fortran standard.
0901 Use of the "$" character in identifiers is an extension to the Fortran standard.
0902 The char_len.fld is Null_Idx in create_runtime_array_constructor.
0903 Constant length character array constructor values with unequal lengths are nonstandard.
0904 Call_Opr in constant constructor that was not an intrinsic.
0905 The section subscript triplet produces a subscript value greater than the declared upper bound.
0906 A structure constructor in a DATA statement value list must represent a constant value.
0907 All bounds and kind selector specification expressions must be scalar.
0908 This integer constant is too large to be used in a 46-bit multiply or divide.
0909 There are not enough registers available to make this vfunction call.
0910 Arguments of derived type or type character are not allowed for vfunction calls.
0911 An assembly language file (-eS or -S filename) disables an assembly language listing (-rg).  Output file selected.
0912 sgif90-912: %d ERRORS found in command line.  Compilation aborted.
0913 A binary output file must be enabled to get an assembly language listing. (-rg)
0914 "%s" has the DIMENSION attribute, therefore it must have an explicit interface to be used as a function.
0915 "%s" has the POINTER attribute, therefore it must have an explicit interface to be used as a function.
0916 "%s" has a variable length character result, therefore it must have an explicit interface.
0917 The CIF option has an illegal argument "%s".
0918 Function "%s" has a pointer result, but the result has not been pointer assigned or allocated.
0919 "%s" has been host associated and used as a named constant, therefore it must not be redeclared as an internal %s.
0920 "%s" is host associated, therefore it must not be redeclared with the %s attribute.
0921 "%s" is host associated, therefore it must not be redeclared as a %s.
0922 "%s" has been use associated, therefore it must not be redeclared with the %s attribute.
0923 "%s" has been use associated, therefore it must not be redeclared as a %s.
0924 "%s" is host associated into this scope, so it cannot be redeclared as variable length character.
0925 "%s" has been use associated, therefore it must not be redeclared as variable length character.
0926 "%s" is host associated, therefore it must not be redeclared with the DIMENSION attribute.
0927 "%s" has been use associated, therefore it must not be redeclared with the DIMENSION attribute.
0928 "%s" is host associated, therefore it must not be redeclared in a common block.
0929 "%s" has been use associated, therefore it must not be redeclared in a common block.
0930 "%s" is host associated, therefore it must not be redeclared as a function.
0931 "%s" has been use associated, therefore it must not be redeclared as a function.
0932 "%s" is host associated, therefore it cannot be redeclared as a subroutine.
0933 "%s" has been use associated, therefore it must not be redeclared as a subroutine.
0934 Module "%s" has no public entities declared in the module, therefore names specified in the rename-list cannot be found.
0935 The integer arithmetic expression exceeds 46-bit maximum.  64-bit integer arithmetic will be tried.
0936 A DATA implied-DO loop control expression must be a scalar integer expression.
0937 The compiler is trying to use the intrinsic enum to call an intrinsic processor, but the enum is Unknown_Intrinsic.
0938 A real division was encountered in an expression being converted to integer.
0939 Function "%s" is typed as assumed-length character.  It must have a known character length to be called.
0940 "%s" is a statement function dummy argument and a variable.  The types must agree.
0941 Common block "%s" is defined and/or associated as a standard and a task common block in the same scope.
0942 Common block "%s" is defined and/or associated in this scope as a standard and an auxiliary storage block.
0943 The use of type default real or double precision real expressions for io-implied-do-control is an obsolescent feature.
0944 The use of a type default real or double precision real DO variable is an obsolescent feature.
0945 An auxiliary variable must not be used in an I/O statement.
0946 If a SIZE= specifier is present in an I/O control list, an ADVANCE= specifier also must appear.
0947 The array actual argument cannot be optional in this context.
0948 The "%s" option was specified so the compilation is aborting on the first error.
0949 Generic interface "%s" must not have an explicit type.
0950 "%s" is a generic interface or an intrinsic.  It must not be typed, because it has already been referenced.
0951 The extent for dimension %d is too large for array "%s".
0952 "%s" is the name of a local variable in a host scope so it must not be the name of an external procedure.
0953 "%s" is the name of a local entity in a host scope so it must not be the name of the module in a USE statement.
0954 SGI sgif90 Version %s (%s) %s
0955 Program "%s" is missing an END statement.
0956 "%s" is host associated into this scope, but is not a derived type so it must not be used as a derived type.
0957 Permitting a BOZ constant to be signed is an extension to the Fortran standard.
0958 A sign is permitted only for an integer literal constant or a real literal constant.
0959 The AUTOSCOPE parameter on a PARALLEL or DOALL compiler directive is not yet supported.
0960 Variable "%s" is in a parallel region and must be in the shared or private list if autoscope is not specified.
0961 The DO control variable must not be SHARED in a DOALL loop.
0962 An array constructor implied-DO control expression must be a scalar integer expression.
0963 Encountered SH_Tbl_Idx in cmp_ref_trees.
0964 Non constant character in gen_static_dv_whole_def.
0965 Problem in process_deferred_functions.
0966 Nonconstant shape for create_bd_ntry_for_const.
0967 Nonconstant character len in create_bd_ntry_for_const.
0968 Assumed_Size func result in set_shape_for_deferred_funcs.
0969 Non constant character in gen_internal_dope_vector.
0970 No function type information in call_list_semantics.
0971 Trying to change data obj in call_list_semantics.
0972 Bad dummy argument in call_list_semantics.
0973 Improper ir for left side of ptr assignment.
0974 Wrong statement header in assignment_stmt_semantics.
0975 Improper ir tree in expr_semantics.
0976 Expected Dv_Deref_Opr from ptr_assign_from_pointer.
0977 Expected ir from ptr_assign_from_pointer.
0978 Something other than list opnd in array_construct_semantics.
0979 Bad assumption in fold_aggragate_expression.
0980 Unexpected type for lcv constant in interpret_constructor.
0981 Not constant IN interpret_constructor.
0982 Not an lcv constant in interpret_constructor.
0983 Invalid position idx in interpret_constructor.
0984 Structure in array syntax portion of interpret ref.
0985 Error in create_array_constructor_asg, not ir idx.
0986 Error in create_array_constructor_asg, unexpected opr.
0987 Problem in create_struct_constructor_asg.
0988 "%s" is the result name of function "%s", therefore it must not be use associated from module "%s".
0989 The compiler has found an unexpected entity "%s" in the attr table when checking for not visible during use processing.
0990 It is illegal to take the LOC of an auxiliary array.
0991 Multiple specific interfaces exist for "%s" and are ambiguous.  This makes %s interface "%s" ambiguous.
0992 "-O %d" is specified, therefore "-O %s%d" must not be specified.
0993 An operator internal text does not have a valid type table index.
0994 The actual arguments to the ASSOCIATED intrinsic may both be dummy arguments only if they are both pointers.
0995 Unrecoverable error encountered while attempting to print buffered messages - %s
0996 The value of this subscript is greater than the declared upper bound.
0997 The section subscript triplet produces a subscript value less than the declared lower bound.
0998 The stride value of a section subscript triplet must not be zero.
0999 The argument to the CHAR intrinsic is outside the valid collating sequence.
1000 Reserved
1001 The stride of an array subscript triplet must not be zero.
1002 "%s" is specified in a namelist group, therefore the type is assumed to be %s.
1003 This compilation unit contains more than one unnamed program unit.
1004 "%s" has a negative offset.  (attr index is %d).
1005 "%s" is a common block name and is also declared as an intrinsic procedure.
1006 Global name "%s" is in use as a common block name and a %s name.
1007 Global name "%s" is in use as a %s name and a %s name.
1008 "%s" cannot be double aligned because of equivalence alignment.
1009 This compilation unit contains multiple main program units.
1010 Global name "%s" is in use already as a %s name.
1011 Procedure "%s" and this compilation must both be compiled with -a dalign, or without -a dalign.
1012 The DIM argument to the LBOUND, UBOUND, or SIZE intrinsic must be a value between 1 and the rank of the ARRAY argument.
1013 The -a dalign command line option has forced padding to be inserted before "%s" in common block "%s".
1014 Character object "%s" has nonconstant bounds.  It can only be declared in a function, subroutine or interface body.
1015 Local-name "%s" must only be referenced once in a rename-list in this scope.
1016 An invalid type "%s" was sent to arith input conversion.
1017 The DIM argument is not in a valid range.
1018 The char_len field is not set for a character expression in create_tmp_asg.
1019 "%s" is in an equivalence group, but does not have ATD_EQUIV set TRUE.
1020 Internal compiler error in process_variable_size_func.
1021 The implied-DO was transformed into a whole array or array section initialization.
1022 Expected character substring in create_io_call_descriptor.
1023 The line number passed on the fei_new_stmt must be nonzero.
1024 Expression evaluation failed. 
1025 Nonconstant character length in create_scalar_type_tbl.
1026 Cannot open output file for module information table for module "%s".  Check directory permissions.
1027 Module "%s", specified on the USE statement, is the module currently being compiled.  A module cannot use itself.
1028 "%s" is a host associated common block name and is an intrinsic procedure.  This is nonstandard.
1029 "%s" is a use associated common block name and is an intrinsic procedure.  This is nonstandard.
1030 "%s" is a host associated common block name and is also declared as an intrinsic procedure.
1031 "%s" is a use associated common block name and is also declared as an intrinsic procedure.
1032 "%s" is a named constant and is also a host associated common block name.  This is nonstandard.
1033 "%s" is a named constant and is also a use associated common block name.  This is nonstandard.
1034 The compiler cannot open file "%s [or .o]", which may contain information about used module "%s".
1035 Character object "%s" is used recursively to declare its own length.
1036 Array "%s" is used recursively to declare its own bounds.
1037 "%s" has been declared and used as a dummy argument, therefore it must not be declared or used as a procedure.
1038 "%s" has been declared and used as a dummy argument, therefore it must not be declared as a %s.
1039 "%s" has been declared and used as a dummy argument, therefore it must not be declared with the %s attribute.
1040 "%s" has been declared and used as a dummy argument, therefore it must not be used as a derived-type name.
1041 Dummy procedure "%s" is called within a parallel region and must be in the shared list if autoscope is not specified.
1042 The compiler could not successfully write intermediate file "%s".
1043 The compiler cannot open intermediate file "%s".
1044 Internal : %s
1045 sgif90: SGI sgif90 Version %s (sgif90:%s mif:%03d) %s
1046 The bounds needed for automatic variable "%s" are not available at all entry points.
1047 Typeless or BOZ constant is not allowed because the DO-variable is type DOUBLE PRECISION.
1048 Invalid operator encountered in change_to_base_and_offset.
1049 Invalid form to operator Init_Reloc_Opr found in cvrt_exp_to_pdg.
1050 Invalid form of Implied_Do_Opr in expand_io_list.
1051 Object "%s" is data initialized, but does not have a storage type of static or common.
1052 The TARGET environment variable has not been set.
1053 "%s" is both a MODULE name specified on a USE statement and an object use associated from MODULE "%s".
1054 A 32 bit integer is not a valid argument to this intrinsic.
1055 Module file "%s" is incompatible with this compiling system.  Recompile the module with this compiling system.
1056 This is an invalid value for the NCOPIES argument.
1057 Object "%s" has been USE associated twice from the same module but is not the sma object.
1058 Unexpected input to set_merge_link routine during use statement semantics.  Attribute "%s" has 3 indexes - %d %d and %d.
1059 Generic interface "%s" has both subroutine and function specific interfaces.
1060 Converting message file; libcif returned error status:  %s
1061 Integer length %d is not supported on this hardware.  This directive is ignored.
1062 The argument is not in the valid range for this intrinsic.
1063 Index %d is not a valid index for the %s table.
1064 Optimization levels "%s" and "%s" are specified on the command line.  These cause the default "%s" to be reset to "%s".
1065 Object "%s" has already been specified in a CACHE_ALIGN directive.
1066 Variable subobjects are not allowed in a CACHE_ALIGN compiler directive.
1067 This object is not allowed in a CACHE_ALIGN compiler directive.
1068 Optimization level "%s" is specified on the command line.  This causes the default "%s" to be reset to "%s".
1069 The %s optimization option is ignored, if the debugging level is %d.
1070 This reference to the "%s" intrinsic must have an integer scalar result to be specified in a specification expression.
1071 Interface %s(%s) contains procedure "%s" which has dummy argument "%s".  This dummy argument must not be OPTIONAL.
1072 Interface OPERATOR(%s) contains procedure "%s" which has dummy argument "%s".  This dummy argument must have INTENT(IN).
1073 Interface %s(%s) contains procedure "%s" which has dummy procedure "%s".  A dummy procedure is not allowed.
1074 Interface ASSIGNMENT(=) contains procedure "%s" which has dummy argument "%s".  The dummy argument must have INTENT(%s).
1075 This argument may not be ALLOCATABLE or POINTER.
1076 Arguments which have TYPELESS type are not standard.
1077 Unused
1078 This use of generic interface "%s" is not valid.
1079 Invalid type sent to arith.a folding routine.
1080 The argument to the PRESENT intrinsic must be an unqualified name.
1081 Name qualification is not allowed in this context.
1082 The iteration count is too large.
1083 The magnitude of the final value of the DO variable is too large.
1084 An implied-DO increment value of zero is not allowed.
1085 Namelist group "%s" has the PUBLIC attribute so namelist group object "%s" cannot have PRIVATE components.
1086 More than one actual argument has been specified for argument "%s" of the %s intrinsic.
1087 Argument "%s" to the %s intrinsic has the wrong rank.
1088 Argument "%s" to the %s intrinsic has the wrong type.
1089 Argument "%s" to the %s intrinsic has the wrong kind type.
1090 The %s intrinsic requires the "%s" argument.
1091 The rank of this POINTER actual argument does not match the rank of POINTER dummy argument "%s".
1092 SGI pointer "%s" has multiple pointees which are typed with differing numeric lengths.
1093 The output argument of this intrinsic does not conform to the other actual arguments.
1094 "%s" is declared as the statement function dummy argument, therefore it must not be used as an external function name.
1095 Invalid form of preparsed format constant.
1096 An assumed-size array is illegal in this context.
1097 It is nonstandard to mix kind types in an equivalence group, if any object in the group is a non default numeric type.
1098 "%s" is a MODULE name specified on a USE statement, so it must not be specified as a rename or only name.
1099 An INTEGER variable has been used as a FORMAT specifier but was not specified in an ASSIGN statement.
1100 I/O list items of a derived type which have a PRIVATE component are not allowed.
1101 An identifier in a DATA value list must be the name of a scalar named constant.
1102 SGI pointer "%s" has multiple pointees with differing types.  This may be non portable.
1103 Procedure "%s" has the RECURSIVE attribute, so -ev (saveall option) will be ignored for this procedure.
1104 The -eR option overrides the -ev option.
1105 The unroll directive must be followed by EOS or a positive integer value <= 1024,  to be used for the unroll count.
1106 The SHAPE argument must have a constant size and must be less than 8.
1107 Function "%s" must be the current function being compiled or a dummy procedure to be typed as assumed size character.
1108 The type of the actual argument, "%s", does not match "%s", the type of the dummy argument.
1109 Initializing a member of blank common is nonstandard.
1110 DOUBLE PRECISION is not supported on this platform.  REAL will be used.
1111 The external name following the equal on the compiler directive NAME must be a character literal.
1112 Invalid %s for %s. It must be a variable, array element or whole array reference.
1113 The %s specifier must be scalar.
1114 Defines "%s" and "%s" are incompatible.  This compiler has not been built correctly.
1115 The maximum number, %d, of alternate entries in one scope has been exceeded.
1116 Defines "%s" or defines "%s" must be specified in defines.h under the appropriate build section.
1117 Internal error in create_struct_argchck_tbl.
1118 The TASK COMMON statement is not supported on this platform.
1119 Internal compiler error. The number of actual arguments does not match the number of dummy arguments for "%s".
1120 This is not a legal value for the DIM argument.
1121 The NUMCPUS tasking directive is not allowed within a parallel region.
1122 The value specified on the NUMCPUS tasking directive is "%d".  Its range must be 1 to 64 so it has been reset to "%d".
1123 Arguments to the CNCALL tasking directive are ignored.
1124 The NUMCPUS tasking directive must have a value specified in the range of 1 to 64.
1125 Invalid type of argument for VAL intrinsic.
1126 Object "%s", specified on the PERMUTATION tasking directive, must be an integer array.
1127 Bit value truncated in conversion.
1128 Common block "%s" is specified in a COMMON directive, therefore it must be specified in a COMMON statement.
1129 Common block "%s" may be specified in a COMMON directive or as TASK COMMON, but it must not be specified as both.
1130 The INTEGER(KIND=6) and INTEGER*6 declarations will be removed in the next release.
1131 The "-i 46" option will be removed in the next release of sgif90.  Use "-O fastint" instead.
1132 File "%s" : "%s".  The compiler is attempting to open this file to search for module "%s".
1133 This Hollerith constant is longer than 256 bits.
1134 "%s" is already a symbolic constant, therefore it must not be given the %s attribute.
1135 "%s" is already a symbolic constant, therefore it must not be declared as a %s.
1136 "%s" is a symbolic constant, therefore it must not be declared as a procedure or used as a derived type.
1137 %s" is a scalar integer symbolic constant, therefore it must not be an array, typed as character, or be in common.
1138 "%s" is a symbolic constant, therefore it must not be an object in a namelist group.
1139 Only one work distribution must be specified on a DO PARALLEL directive.
1140 Work distribution parameter NCPUS_CHUNKS is not supported on this platform.
1141 Only variable names are allowed in a %s compiler directive.
1142 The STACK directive must not be specified in the specification part of a MODULE.
1143 The STACK directive must not be specified inside an interface body or an interface block.
1144 A SAVE statement with no save entity list and a %s directive are specified in the same scope.  SAVE takes precedence.
1145 The result of 128 bit floating point operations currently may be of a lesser precision.
1146 The !DIR$ INTEGER=46 directive will be removed in the next release; "-O fastint" will provide equivalent behavior.
1147 The INLINE ALWAYS and the INLINE NEVER directives have both been specified for object "%s".  Only one may be specified.
1148 Can not build the message file.
1149 N$PES has been referenced, but was not given a value with the -X option.  A value of 0 (zero) is being used for N$PES.
1150 Line number is invalid.
1151 Module "%s" has been referenced in a USE statement.  It may not be redefined with a MODULE statement.
1152 The size of the first (or only) dimension of MATRIX_B must equal the size of the last (or only) dimension of MATRIX_A.
1153 Invalid argument to the PACK intrinsic.
1154 Invalid FIELD argument to the UNPACK intrinsic.
1155 The MASK argument must be conformable with ARRAY.
1156 Illegal value for the POS argument.
1157 Module file "%s" was created with a previous compiler release.  It will not be supported by the next major release.
1158 An ELSE IF block must not follow an ELSE block.
1159 The library format parser is issuing message number "%d".  This message is unknown to the compiler.
1160 The NULL edit descriptor is an extension to the Fortran standard.
1161 Alignment of variable "%s" in common "%s" may cause performance degradation.
1162 -i 46 means -i %d on this platform.  -i 46  will be removed in the next release of this compiler.  
1163 !DIR$ INTEGER=46 means INTEGER=%d on this platform.  The !DIR INTEGER=46 directive will be removed in the next release.
1164 Internal compiler error in conditional compilation routine "%s".
1165 Conditional compilation has unexpected syntax.  Expected %s.
1166 Conditional compilation blocking error.  %s.
1167 Conditional compilation error.  Identifier "%s" is referenced but not defined.
1168 Common block "%s" is specified in a CACHE_ALIGN directive, therefore it must be specified in a COMMON statement.
1169 The !DIR$ [NO]MODINLINE must be specified within the scope of a module.
1170 The %s directive will no longer be available in the next release.
1171 An explicit type must be specified for object "%s", because -eI, the IMPLICIT NONE command line option is specified.
1172 -s cf77types is for FORTRAN 77 programs only.  If used with Fortran 90/95 programs, unexpected behavior may result.
1173 The value "%s" has been used as an align to size in align_bit_length.  Only 8, 16, 32 and 64 are valid bit sizes.
1174 The dope vector element length of %d has exceeded the maximum length of %d.
1175 Integer overflow has occurred while calculating array bounds, character length or offset.
1176 The SHAPE array for the RESHAPE intrinsic function contains a negative value.
1177 Conditional compilation #error:  \"%s\".
1178 The conditional compilation ERROR directive must not be continued.
1179 Internal compiler error. Invalid use of %s in folder_driver call.
1180 "%s" has already been named as a pointee in a POINTER statement.  It cannot be given character type now.
1181 Module file "%s" must be recompiled. Its format is unsupported.
1182 "-O taskinner" is specified, therefore "-O task2" or "-O task3" must be specified.
1183 Module file "%s" must be recompiled. It was created by a later release of the compiler
1184 Evaluation of this constant expression produced a NaN or other abnormal value.
1185 The "-O pattern" option is temporarily disabled.
1186 The actual argument to IACHAR and ICHAR must have length equal to one character.
1187 The SHAPE argument specified is not valid for the SOURCE argument provided.
1188 The arguments to this bitwise intrinsic or operator must be the same size.
1189 This numeric constant is out of range.
1190 Internal compiler error. Invalid type or constant sent to cvrt_str_to_cn.
1191 Unsupported operator in expr_sem.
1192 The "%s" option conflicts with the "%s" option.  The "%s" option is ignored.
1193 Module "%s" is indirectly used in module "%s" and is from file "%s".  That file no longer exists.
1194 The use of the Dw.dEe edit descriptor is an extension to the Fortran standard.
1195 Possible function result type conflict. -i32 changes the type of "%s" to a half word integer.  "%s" is not changed.
1196 Command line option "%s" is being replaced by "%s".  "%s" will no longer be available in the next release.
1197 The array subscript for dimension %d does not fall within the defined range.
1198 Alignment of component "%s" in numeric sequence structure "%s" may cause performance degradation.
1199 Inlining may not be combined with debugging.  Inlining has been turned off.
1200 An invalid name is being entered into a name table.  The name must not be NULL and must be greater than one in length.
1201 The offset or length for block or object "%s" must be constant.  It is not.
1202 Routine %s was not inlined because %s.
1203 Bad assumption in create_constructor_constant.
1204 The call to %s was inlined.
1205 Dummy argument "%s" is longer than the actual argument.
1206 Asterisk is not allowed as UNIT control item specifier for this %s statement.
1207 An asterisk can not be used as the UNIT control item specifier for unformatted I/O.
1208 A FMT or NML specifer without a keyword following a keyworded UNIT specifier is an extension to the Fortran standard.
1209 Routine %s was conditionally expanded inline.
1210 Subprograms that contain assigned GOTO statements cannot contain Autotasking directives.
1211 The character length for "%s" is dependent upon the runtime N$PES symbolic constant.  This is not allowed.
1212 An initialization expression must not contain symbolic constant "%s".
1213 "%s" for "%s" must be a constant or a symbolic constant tmp.
1214 INTENT OUT dummy arguments must not have a constant passed as an actual argument.
1215 Command line option "-du" must not be specified with "-O ieeeconform".  "-eu" is in effect.
1216 Command line option "-O ieeeconform" causes default option "-du" to be switched to "-eu".
1217 The %s autotasking directive is missing for this %s directive.
1218 Implementation of command line option "%s" is deferred.
1219 This %s directive does not precede a DO loop construct.
1220 Illegal branch %s a %s autotasking region.
1221 The -O command line option has been specified with no arguments.  It will be ignored.
1222 The FIELD argument to the UNPACK intrinsic does not conform to MASK.
1223 "%s" has the %s attribute, so it must not be an array whose bounds are determined by a symbolic constant expression.
1224 "%s" is a %s, therefore it must not be an array whose bounds are determined by a symbolic constant expression.
1225 Character constant intrinsic argument has been converted to Hollerith.
1226 The "-ea" option was specified so the compilation is aborting on the first error.
1227 "%s", in common block "%s", must not be initialized, because the block length is dependent on a symbolic constant.
1228 "%s", in common block "%s", must not be equivalenced, because the block length is dependent on a symbolic constant.
1229 The use of symbolic constant "%s" is non standard.
1230 Entry point "%s" is an array based on symbolic constants.  There are multiple entry points, so this is not allowed.
1231 Command line options -Xm and -X # (where # is number of N$PES) are mutually exclusive.  -X %s takes precedence.
1232 Command line option -Xm has been specified, so N$PES must not be used in a declaration statement.
1233 The %s directive must not be specified in the specification part of a MODULE.
1234 The SYMMETRIC directive does not apply to variable "%s" because the variable has the "%s" attribute.
1235 Variable "%s" must not be specified with the !DIR$ SYMMETRIC directive because it is host associated.
1236 SYMMETRIC does not apply to variable "%s" because it is host associated.
1237 I/O problem in module processing.  I/O error is "%s".
1238 "%s" is an invalid argument to the -X command line option.  The argument to -X must be m or in the range 1 thru 2048.
1239 All objects in this equivalence set must be default numeric or numeric sequence derived type.  "%s" is not.
1240 All objects in this equivalence set must be default character or character sequence derived type.  "%s" is not.
1241 Object "%s" is a non-default intrinsic type.  All objects in this equivalence set must be the same type and kind type.
1242 "%s" is a not a character or numeric sequence derived type, so all objects in the equivalence set must be the same type.
1243 The .LG. operator is an extension to the Fortran standard.
1244 "%s" is type(%s) whose components are typed using -s default32.  Equivalence storage association may not be as expected.
1245 Object "%s" was not declared with co-dimensions.
1246 Precompiled procedure "%s" is incompatible with this compiling system.  Recompile with this compiling system.
1247 Precompiled procedure "%s" must be targeted for the same operating system as this compilation.
1248 Procedure "%s" and this compilation must both be compiled with "%s", or both must be compiled without the option.
1249 Module "%s" was compiled in this compilation, but the compiler can no longer find the module.
1250 Expected to find "%s" in the global name table, but srch_global_name_tbl returned not found.
1251 The optional pad amount specified on the -apad[n] command line option must be between 0 and 4096.  %d is specified.
1252 The binary output file "%s" is the same as the binary output file created for inlinefrom file "%s".
1253 The %s statement is an extension to the Fortran standard.
1254 The %s attribute is an extension to the Fortran standard.
1255 "%s" must not have the AUTOMATIC attribute as it is an array, pointer, character or a derived type function result.
1256 Object "%s" has the %s attribute, so it must not be equivalenced to an object in a common block.
1257 Object "%s" has the %s attribute, so all objects in this equivalence group must have the %s attribute.
1258 Module "%s" contains objects whose type is integer(kind=6) or integer*6.  This is no longer supported.
1259 "%s" has been given the %s attribute more than once.  This is nonstandard.
1260 Prefix-spec %s is specified twice for this subprogram.
1261 RECURSIVE and ELEMENTAL must not be specified for the same subprogram.
1262 A %s statement is not allowed within %s subprogram "%s".
1263 A %s statement whose io-unit is an external-file-unit or *, is not allowed in %s subprogram "%s".
1264 Variable "%s" in %s subprogram "%s" must not be given the %s attribute.
1265 Non-pointer dummy argument "%s" to %s FUNCTION "%s" must have INTENT(IN) specified for it.
1266 Non-pointer dummy argument "%s" to %s SUBROUTINE "%s" must have an intent specified for it.
1267 "%s" is a dummy argument to elemental subprogram "%s", so it must be a non-pointer scalar dummy argument.
1268 "%s" is an elemental function, so its function result "%s" must be scalar and cannot be a pointer.
1269 An alternate return specifier is not allowed as a dummy argument to elemental SUBROUTINE "%s".
1270 "%s" must not be defined inside of a %s subprogram.  It is in common, a dummy argument or host or use associated.
1271 "%s" is a dummy procedure to pure subprogram "%s".  It must be specified with the PURE attribute.
1272 "%s" is an internal subprogram to %s subprogram "%s".  It must be given the %s prefix-spec.
1273 "%s" must not be argument associated with "%s" in a %s subprogram.
1274 Procedure "%s" must be %s, because it is referenced in a %s subprogram.
1275 Common block "%s" is used in multiple program units and has different lengths in the different program units.
1276 Common block "%s" is used in multiple program units.  It must be in %s storage in all uses.
1277 Procedure "%s" is %s at line %s.  It must have an explicit interface specified.
1278 Procedure "%s" is defined at line %s.  Dummy argument "%s" is scalar.  This argument is an array argument.
1279 Procedure "%s" is defined at line %s.  The type of this argument does not agree with dummy argument "%s".
1280 Directive "%s" is being replaced by "%s".  "%s" will no longer be available in the next release.
1281 This use of the COPY_ASSUMED_SHAPE directive is not allowed.
1282 "%s" is defined as a %s at line %s and as a %s in this program unit.
1283 Global name "%s" is declared in an interface block as a %s at line %s, but is defined as a %s in this program unit.
1284 Procedure "%s" is defined at line %s and has %d dummy argument(s).  This interface has %d dummy argument(s) specified.
1285 The %s attribute is specified in either the program unit or interface block definition for "%s", but not in both.
1286 Type and/or rank for dummy argument "%s" to %s "%s" defined at line %s does not agree with this interface block.
1287 Type and/or rank for result "%s" to function "%s" defined at line %s does not agree with this interface block.
1288 Global name "%s" is defined as a %s at line %s, but declared in this interface block as a %s.
1289 A %s directive cannot be specified within a DO PARALLEL loop.
1290 Type and/or rank for result "%s" to function "%s" in the interface block at line %s differs with this definition.
1291 Type and/or rank for dummy argument "%s" to %s "%s" in the interface block at line %s differs with this definition.
1292 The interface block for procedure "%s" at line %s has %d dummy argument(s).  The definition has %d dummy argument(s).
1293 Program unit "%s" is defined as a %s at line %s, but is defined or referenced as a %s here.
1294 Function "%s" is defined at line %s. Type and rank for result "%s" differ at this definition or reference.
1295 "%s" is defined or referenced at line %s and here.  The number of arguments do not match.  Expected %d, but found %d.
1296 Subroutine "%s" defined at line %s has an alternate return dummy argument, so this actual argument must be a user label.
1297 Procedure "%s" is defined at line %s with dummy procedure "%s".  This argument is a data object, constant or expression.
1298 Procedure "%s" is defined at line %s. The type and rank of dummy function "%s" must agree with procedure argument "%s".
1299 "%s" is defined/referenced at line %s with %s "%s" as an argument. This argument must be the same type of program unit.
1300 "%s" is referenced at line %s and defined here. Argument "%s" is a dummy procedure. The actual argument is not.
1301 Procedure "%s" is also referenced at line %s.  The type of argument %d does not agree.
1302 An assumed size character array used as an internal file is an extension to the Fortran standard.
1303 Only assumed shape dummy arguments are allowed on the COPY_ASSUMED_SHAPE directive.
1304 The COPY_ASSUMED_SHAPE directive was encountered but there are no assumed shape dummy arguments.
1305 This character literal has been blank padded to the length of the dummy argument.
1306 Blank padding of a character literal actual argument is an extension to the Fortran standard.
1307 The kind (%d) of this actual argument does not match that of its associated dummy argument (%d).
1308 In a real constant with a D, E or Q following the decimal field, the D, E or Q must be followed by an exponent.
1309 The kind parameter suffix is not valid on a double or quad precision real constant form.
1310 f90: Compile time:  %s seconds
1311 This array argument to an ELEMENTAL call is not conformant to the other arguments.
1312 This DO PARALLEL directive is not allowed within a parallel CASE region.
1313 Command line option "%s" overrides "%s". 
1314 Object "%s" is declared both %s and %s.
1315 An internal inlining data structure has overflowed.   The compilation cannot continue.
1316 Symbol table field %s is NULL and should not be at this location.
1317 SAFEVL is not allowed on the IVDEP directive on this platform.  This IVDEP directive will be ignored.
1318 "%s" is specified on the CACHE_BYPASS directive.  It must have the dimension attribute.
1319 An array name must be specified on the CACHE_BYPASS directive.  A subobject or an expression is not allowed.
1320 "%s" is specified on the CACHE_BYPASS directive.  Its type must be integer, logical or real with a kind type of 8.
1321 Attr list table entry %d holds attr entry %d, which is being compressed out.
1322 The compiler has detected errors in procedure "%s".  No inline information file will be created for this procedure.
1323 The kind type of all the arguments to the intrinsic are not identical. 
1324 Maximum number of loops (%d) specified for a given line has been exceeded.
1325 Routine %s was not inlined because a store into a constant will occur.
1326 Routine %s was not inlined because the routine contains an alternate RETURN.
1327 Routine %s was not inlined because the routine calls the PRESENT intrinsic.
1328 Routine %s was not inlined because a corresponding actual and dummy argument do not have identical type and kind-type.
1329 Routine %s was not inlined because the routine calls the NUMARG intrinsic.
1330 Routine %s was not inlined because a scalar actual argument is being mapped to an array dummy argument.
1331 Routine %s was not inlined because the routine contains ASSIGN statements.
1332 Routine %s was not inlined because the routine is RECURSIVE.
1333 Routine %s was not inlined because the routine being called is a dummy procedure.
1334 Routine %s was not inlined because it has OPTIONAL dummy arguments.
1335 Routine %s was not inlined because an INLINE directive is not in effect.
1336 Routine %s was not inlined because the call was not within a loop.
1337 Routine %s was not inlined because it contains a Fortran pointer in static storage.
1338 Routine %s was not inlined because a NOINLINE directive is in effect.
1339 Routine %s is not inlined because an INLINENEVER directive has been specified.
1340 Routine %s was not inlined because a dummy argument of the routine being inlined is a dummy procedure.
1341 Routine %s was not inlined because the routine has a dummy argument that is referenced in a child routine.
1342 Routine %s was not inlined because there are an unequal number of actual and dummy arguments.
1343 Routine %s was not inlined because a maximum of 256 actual arguments can be in the call list.
1344 Routine %s was not inlined because the compiler was unable to locate the routine to expand it inline.
1345 Routine %s was not inlined because the routine references a dummy argument not from the ENTRY taken.
1346 Routine %s was not inlined because there is a USE of a module that has not been defined yet.
1347 Routine %s was not inlined because it is too large to expand inline.
1348 QUAD PRECISION is not supported on this platform. DOUBLE PRECISION will be used.
1349 Constant table entry %d had a CN_POOL_IDX of zero.
1350 The %s directive is obsolete.  Support will be removed in the next major release.
1351 Storage block "%s" will not be padded (-a pad), because one or more objects in the storage block are equivalenced.
1352 The static storage block will not be padded (-a pad), because one or more objects in the storage block are equivalenced.
1353 Command line option conflict detected between "%s" and "%s".  "%s" is selected.
1354 Unimplemented compiler directive: %s.
1355 Routine %s was not inlined because it has dummy argument(s) which are SGI pointers.
1356 Expected a valid compiler directive.
1357 Routine %s was not inlined because this function's result is referenced in a child routine.
1358 Routine %s was not inlined because the routine contains a LOC of a variable in COMMON.
1359 Routine %s was not inlined because it contains SGI pointers which are in COMMON.
1360 More than one %s clause has been specified for this %s directive.
1361 The ONTO clause is not allowed with the %s directive unless a NEST clause is seen first.
1362 Object "%s" is specified in more than one scoping list.
1363 Objects in the REDUCTION clause must be scalar references.
1364 The CHUNK expression must be a scalar INTEGER expression.
1365 Variable "%s" must be declared in a COMMON block in order to use PE dimensions.
1366 References with PE dimensions are not allowed as actual arguments.
1367 Bounds entry %d has a variant problem with field %s.
1368 Arguments to the ONTO clause must be INTEGER constants greater than zero.
1369 The number of ONTO constants must equal the number of NEST variables.
1370 The %s clause is not allowed with the %s mp directive.
1371 The argument to the THREAD clause must be a scalar INTEGER expression.
1372 Arguments to the DATA clause must be array elements.
1373 Arguments to the THREAD clause must be constants.
1374 A subobject reference is not allowed in this context.
1375 The variable name list must contain at least two names.
1376 Objects in the LASTTHREAD clause must be simple scalar variables.
1377 The ONTO clause can only be specifed when the count of objects in the NEST clause is greater than 1.
1378 Invalid argument to the %s directive.
1379 The DO-variable was not specified in the %s list preceding this loop nest. 
1380 %s loops are not perfectly nested. 
1381 A statement must not appear between the %s directive and the DO loop that follows it.
1382 The argument to the PREFETCH_REF_DISABLE directive must be an array name.
1383 The SIZE argument to the %s directive must be a constant.
1384 Invalid LEVEL value for this PREFETCH_REF directive.
1385 The autotasking directive %s must be specified within a do loop.
1386 The second argument to the %s directive must be L1cacheline, L2cacheline, page, or a power of 2 constant.
1387 A(n) %s directive has already been specified for the current loop nest.
1388 Routine %s was not inlined because it is a function with alternate entries.
1389 The DO loop nesting depth does not match the number of DO-variables in the BLOCKABLE directive DO-variable list.
1390 SGI character pointee "%s" will be treated as an assumed-length character variable.
1391 Source file "%s" contains no Fortran statements.
1392 Intrinsics which return a data type other than integer are not allowed in specification expressions.
1393 Initializing multiple array elements with a single Hollerith or character constant will be removed in the next release.
1394 Only COMMON block names and COMMON block variables are allowed as arguments to the COPYIN directive.
1395 The C$COPYIN directive cannot be used within a parallel region.
1396 Only array variable names can be specified on the %s directive.
1397 This must be a scalar INTEGER expression.
1398 Too %s ONTO values.
1399 This intrinsic will not be supported in the 3.2 release and beyond.
1400 An allocatable array is not allowed in this context.
1401 sgif90: %d source lines
1402 sgif90: SGI Pro64 Fortran 90 Version %s (%s) %s
1403 sgif90: %d Error(s), %d Warning(s), %d Other message(s), %d ANSI(s)
1404 The STACK directive must not be specified inside an interface body or an interface block.
1405 The STACK directive must not be specified in the specification part of a MODULE.
1406 Overflow of file_id_list in cif_flush_include_recs.
1407 "%s" has the %s attribute, support for declaration as a %s is deferred.
1408 "%s" is a %s, support for a declaration with the %s attribute is deferred.
1409 Routine %s was not inlined because the routine contains a !DIR$ INTEGER= directive.
1410 The length of the keyword at index %d in the %s table is %d, which is greater than the maximum of 31.
1411 The %s directive cannot be specified within a %s.
1412 The %s directive must be specified within a %s.
1413 This numeric constant is out of range.
1414 The symbolic constant "N$PES" is an extension to the Fortran standard.
1415 The AUTOSCOPE clause is not supported on this platform.
1416 The order of the loop indices must correspond to the order in this NEST clause.
1417 This AFFINITY list item does not match the NEST list.
1418 ONTO is not allowed for DATA affinity.
1419 Array "%s" is an assumed-size pointee.  It must not be declared in a module.
1420 An interface body cannot access named entities from the host, thus derived type "%s" is undefined.
1421 Local name entry %d has been compressed out during interface compression.  It should not be.
1422 The SAFE_DISTANCE= value on the CONCURRENT directive must be a scalar integer constant greater than or equal to one.
1423 Implicit typing is confirmed for object "%s".  The Fortran standard requires typing before reference with IMPLICIT NONE.
1424 A type must be declared for "%s" before it is referenced, initialized or declared a constant because of IMPLICIT NONE.
1425 Routine %s was not inlined because the function result types do not match.
1426 "%s" has been referenced or defined in a prior statement in this scope.  It may not be given the PARAMETER attribute.
1427 "%s" is a SGI character pointer.  All its pointees must be character, but "%s" is not character.
1428 "%s" is a SGI pointer.  All its pointees must be non-character, but "%s" is typed as character.
1429 The %s command line option overrides the %s directive, so the directive is ignored.
1430 %s array "%s" cannot be declared in the LOCAL clause of this %s directive.
1431 The argument to the %s clause of the %s directive must be an integer constant or variable.
1432 The argument to the %s clause of the %s directive must be an integer constant.
1433 The argument to the %s clause of the %s directive must be a logical expression.
1434 The storage size needed for this expression exceeds %s bytes, the maximum storage size available.
1435 The storage size needed for "%s" exceeds %s bytes, the maximum storage size available.
1436 The %s clause is not supported and will be ignored.
1437 Routine %s was not inlined because a LOC() of a constant will be generated.
1438 This argument produces a %s to a temporary variable.
1439 Dummy argument "%s" must not appear in a specification expression.
1440 Common block name "/%s/", not the object name "%s" must be specified on the %s directive.
1441 "%s" has the %s attribute, therefore it must not be specified with compiler directive %s.
1442 "%s" is a %s, therefore it must not be declared again with compiler directive %s.
1443 "%s" has been declared and used as a dummy argument.  It must not be declared again with compiler directive %s.
1444 "%s" has been used as a variable, therefore it must not be declared again with compiler directive %s.
1445 The %s directive does not affect autotasking under the -pfa command line option.
1446 %s variable "%s" cannot be declared in the PRIVATE clause of this %s directive.
1447 "%s" is already a symbolic constant, therefore it must not be specified with compiler directive %s.
1448 "%s" has been use associated, therefore it must not be declared again with compiler directive %s.
1449 "%s" is host associated, therefore it must not be specified with the %s compiler directive.
1450 "%s" is typed as variable length character, therefore it must not be declared again with compiler directive %s.
1451 "%s" is an explicit-shape array with nonconstant bounds, therefore it must not be specified with compiler directive %s.
1452 "%s" is specified with the %s directive, therefore it must not be typed as variable length character.
1453 "%s" is defined in an explicit interface, therefore it must not be declared again with compiler directive %s.
1454 "%s" has been used as a function, therefore it must not be declared again with compiler directive %s.
1455 "%s" has been used as a subroutine, therefore it must not be declared again with compiler directive %s.
1456 Only elemental intrinsics of type integer or character are allowed in an initialization expression.
1457 "%s" is specified with the %s directive, so it must not be declared again with the %s directive.
1458 "%s" is specified with the %s directive, so it must not be declared as a %s.
1459 "%s" is specified with the %s directive, therefore it must not be specified with the %s attribute.
1460 A call to start_pes_() has been generated by the compiler.
1461 This use of construct name "%s" is not allowed.
1462 This use of label "%s" is not allowed.
1463 "%s", specified with the %s directive, is a procedure.  Statement function actual arguments must be scalar data objects.
1464 "%s" is specified with the %s directive.  It must not be declared as an explicit-shape array with nonconstant bounds.
1465 "%s" is specified with the %s directive.  It must not be an array whose bounds are dependent on a symbolic constant.
1466 "%s" is specified with the %s directive, therefore it must not be a procedure in an interface block.
1467 "%s" is specified with the %s directive, therefore it must not be used as an external function name.
1468 "%s" is specified with the %s directive, therefore it must not be used as a subroutine name in a CALL statement.
1469 "%s" is specified with the %s directive, therefore it must not be used as a derived-type name.
1470 "%s" is specified with the %s directive, so it is not a constant.  It must not be used in an initialization expression.
1471 "%s" has been referenced or declared in this scope already.  It must not be declared as the function result.
1472 This END CRITICAL directive must have the same name as the matching CRITICAL directive.
1473 Object %s must be a variable to be in the %s clause of the %s directive.
1474 DO, SECTIONS, and SINGLE directives are not allowed to be nested.
1475 It is illegal to specify a CHUNK in the SCHEDULE clause when RUNTIME schedule type is specified.
1476 Variable "%s" is specified in more than one !$OMP directive clause.
1477 It is illegal to declare a scope attribute for pointee "%s".
1478 %s, "%s", may not be specified in FIRSTPRIVATE or LASTPRIVATE clauses.
1479 Common block "%s" must be declared before being specified with the THREADPRIVATE directive.
1480 Only named variables are allowed in the list of a FLUSH directive.
1481 Blank common, "//", must not be declared in a %s directive.
1482 %s arrays cannot be specified as PRIVATE, FIRSTPRIVATE, or LASTPRIVATE.
1483 Object %s must be a scalar variable to be in the REDUCTION clause of the %s directive.
1484 %s "%s" is not allowed in the %s clause.
1485 Common block "/%s/" is host or use associated.  It must not be specified with the THREADPRIVATE directive.
1486 Storage block "/%s/" must be declared as a common block before being specified with the THREADPRIVATE directive.
1487 A variable subobject is not allowed on the %s compiler directive.
1488 "%s", a member of common block "/%s/" must not be equivalenced because directive %s has been specified for the block.
1489 "%s", a member of module "%s" must not be equivalenced because directive %s has been specified for the module.
1490 Storage block or module "%s" has the %s directive specified for it, so the %s directive must not also be specified.
1491 If compiler directive %s is specified for a module name, it must be for the current module being compiled.
1492 Dummy argument "%s" has the INTENT(IN) attribute. It must be definable to be declared in a %s clause.
1493 Variable "%s" is in a %s procedure and cannot be defined. It must be definable to be specified in a %s clause.
1494 Variable "%s" is referenced but not defined in this program unit.
1495 Routine %s was not inlined because it is a module procedure and NOMODINLINE is in effect.
1496 "%s" is not a module in this scope, therefore it must not be specified on the implicit use option as a module-name.
1497 "%s" is specified with the %s directive.  It must be a common block name, a module name, or in static storage.
1498 Module name "%s", not the object name "%s" must be specified on the %s directive.
1499 The value for work distribution %s must be a positive integer.
1500 The overall size of the dummy argument array is greater than the size of this actual argument.
1501 "%s" has been referenced in the function character length expression.  It must not be declared with directive %s.
1502 Common block "/%s/" is specified in a %s directive, therefore it must be specified in a COMMON statement.
1503 Illegal branch %s a %s region.
1504 Illegal branch %s a %s region.
1505 "%s" is specified with the %s directive, therefore it must be a dummy argument.
1506 An ORDERED directive can only appear in the dynamic extent of a DO or PARALLEL DO directive.
1507 It is illegal for an ORDERED directive to bind to a DO directive that does not have the ORDERED clause specified.
1508 The %s directive must be specified within the declaration part of a module.
1509 Do loop control variable "%s" is in a parallel region and must be in the PRIVATE list if autoscope is not specified.
1510 Variable "%s" must have its data scope explicitly declared because DEFAULT(NONE) was specified.
1511 The expression in an IF clause must be a scalar Fortran logical expression.
1512 Variable "%s" was privatized in the enclosing PARALLEL region. It cannot be privatized again.
1513 The %s directive must appear immediately after the end of the Do loop.
1514 The DO loop iteration variable must be of type integer when within an OpenMP work-sharing construct.
1515 Procedure "%s" has multiple explicit interfaces declared in the same scope.  This is nonstandard.
1516 Procedure "%s" has multiple explicit interfaces declared in this scope.  The interfaces must agree completely.
1517 Invalid %s clause.
1518 The %s is an extension to the OpenMP standard. It should have the %s sentinal.
1519 The autotasking directive WAIT must not be specified within a %s region.
1520 The autotasking directive %s must be specified within a DOPARALLEL or DOALL region.
1521 All WAIT/SEND pairs within a loop except one must be numbered.  The numbers must be unique.
1522 Object "%s" must not be used as an argument to the KIND intrinsic before being typed.
1523 A WAIT/SEND pair must both have the same POINT value or neither of them should have a POINT value.
1524 COMMON block %s is increasing in length due to the inline expansion of routine %s.
1525 Multiple spans within a loop are not allowed.
1526 The maximum number of wait/send directive pairs for this loop is %d and has been exceeded.
1527 This %s POINT does not match a %s POINT in this loop.
1528 Constant point values must not exceed %d, which is the maximum number of wait/send directive pairs.
1529 Object "%s" is already a module procedure within this module, so it must not be declared again as a module procedure.
1530 The %s directive is obsolete and will be removed in the next major release.
1531 This initialization expression has a non-constant value.  It must evaluate to a constant value at compile time.
1532 If SPAN is specified, it must be an integer constant value in the range 1 to 64.
1533 The argument must be default integer type.
1534 All specifics that are intrinsic, must follow all non-inrinsic specifics.  Intrinsic %s is out of order.
1535 "-O %d" is specified, therefore "-O %s" must not be specified.
1536 The optimzation level should be %s, because the debugging level is %d.
1537 A component reference is not allowed in an EQUIVALENCE statement.
1538 Array "%s" is specified on the -O reshape command line option.  It is not an array in scope "%s".
1539 Array "%s" must be an explicit shape constant or variable sized array to be specified with -O reshape.
1540 The -O inlinefrom= option must be specified when the -O inlinepgm= option is specified.
1541 Support for passing a DISTRIBUTE_RESHAPE array to an assumed-shape dummy argument is deferred.
1542 Initializing a named common block member in a program unit other than a block data is not legal for this architecture.
1543 Routine %s was not inlined because the routine is not a leaf routine in the call graph.
1544 The loop that follows this %s directive cannot be a DO WHILE or a DO loop without loop control.
1545 Routine %s was not inlined because the rank of the actual and dummy arguments are not identical.
1546 Routine %s was not inlined because the extents are not identical in all dimensions.
1547 Module name "%s", not the variable name "%s" must be specified on the %s directive.
1548 %s is no longer the most aggressive form of inlining.  Please see explain for this message number.
1549 Macro "%s" is used within its own definition.
1550 Too few arguments in invocation of macro "%s".
1551 Too many arguments in invocation of macro "%s".
1552 "%s" has the ALLOCATABLE attribute, therefore it must be specified as a deferred-shape co-array.
1553 Routine %s was not inlined because the actual argument is a structure reference.
1554 Routine %s was not inlined because the actual argument is a character constant.
1555 Routine %s was not inlined because it contains DATA statements.
1556 Routine %s was not inlined because the routine contains a parallel region.
1557 Illegal placement of the NULL intrinsic.
1558 This ALLOCATE shape specifier has missing %s.
1559 Pointers must be initialized with the NULL intrinsic.
1560 Invalid CHUNK value.
1561 "%s" is must be a locally declared variable to be specified in the %s list of the %s directive.
1562 Illegal pointer assignment.
1563 The "* <char-length> [,]" form of character declaration is an obsolesent form.
1564 Function "%s" is a %s function.  It must not be declared CHARACTER*(*).
1565 Assumed length character functions are obsolescent.
1566 Assumed length character function "%s" cannot be invoked.  There is no way to match caller/callee characteristics.
1567 Transfer of control to an END IF statement from outside the IF construct is an extension to the Fortran standard.
1568 The %s statement is an extension to the Fortran standard.
1569 A DO loop variable or expression of type default real or double precision real is an extension to the Fortran standard.
1570 The -O stream command line option must not be specifed with the %s command line option.
1571 Positioning the DATA statement amongst executable statements is an obsolescent feature.
1572 Pointer assignment for components of derived type co-arrays is not supported.
1573 The MOLD argument is not a valid argument to the NULL intrinsic in this context.
1574 The MOLD argument must have the POINTER attribute.
1575 This argument to the THIS_IMAGE intrinsic must be a co-array. 
1576 The final co-array bound must be '*'.
1577 "%s" is an automatic variable.  It must not have the co-array DIMENSION attribute.
1578 Variable "%s" has a co-array subobject.  Co-array subobjects must not be specified in %s statements.
1579 Component "%s" of derived type "%s" has the co-array DIMENSION attribute.  This is not permitted.
1580 Pure or elemental subprogram "%s" must not contain "%s", which is a co-array variable or intrinsic.
1581 All statements must be contained within a program unit and followed by END.
1582 Fixed source form is an obsolescent feature in Fortran 95.
1583 Support for %s for %s is deferred.
1584 This actual argument must be a co-array in order to be associated with a co-array dummy argument.
1585 Only variables with a 64 bit type are currently supported in co-array references.
1586 Illegal branch %s a %s region.
1587 Co-array %s must have the ALLOCATABLE attribute in order to have a deferred shape in the co-array dimensions.
1588 The %s statement is not allowed in a parallel or work sharing region.
1589 "%s" must not be specified in a DATA statement.  It is typed as derived type "%s", which is default initialized.
1590 "%s" is an assumed size array with INTENT(OUT).  It must not be typed as %s, because that type is default initialized.
1591 "%s" must not be equivalenced to an object in a common block, as its type is "%s", which is default initialized. 
1592 ntr_stor_blk_tbl has been called with an invalid storage block type.  This should never happen.
1593 Macro expansion has caused a continued statement to exceed the size of an internal source buffer.
1594 In an ALLOCATE statement for a co-array, the upper bound for the final co-dimension must always be '*'.
1595 Transfer of control into the FORALL construct starting at line %d is not allowed.
1596 Label %s is defined in the FORALL construct starting at line %d.
1597 The %s statement is not allowed in a FORALL construct.
1598 The index-name in a FORALL header must be a named scalar variable of type INTEGER.
1599 Index-name "%s" is already an index-name for an enclosing FORALL construct.
1600 Object "%s" is typed as derived type "%s" with default initialization.  It must not also be a common block object.
1601 Routine %s was not inlined because it has a Co-array dummy argument.
1602 Common block "%s" is used in multiple program units.  It must be specified with the %s directive in all uses.
1603 Common block %s is declared in multiple program units.  One or more common objects differ in type, kind or rank.
1604 The subscript or stride in a forall-triplet-spec must be a scalar integer expression.
1605 A subscript or stride must not reference an index-name in the forall-triplet-spec-list in which it appears.
1606 The stride value in a forall-triplet-spec must not be zero.
1607 The scalar-mask-expr for a FORALL statement must be a scalar logical expression.
1608 The FORALL index must not be defined while it is active.
1609 A masked ELSE WHERE block must not follow an ELSE WHERE block.
1610 All mask expression within a WHERE construct must have the same shape.
1611 Procedure %s is referenced within a %s. It must be a PURE procedure.
1612 Routine %s was not inlined because it was called with a Co-array actual argument.
1613 Routine %s was not inlined because it shares a common block with a Co-array member.
1614 Procedure "%s" is also referenced at line %s.  The type and rank of the function result of argument %d do not agree.
1615 Procedure "%s" is defined at line %s.  Dummy argument "%s" is an array argument.  This argument is scalar.
1616 Procedure "%s" is also referenced at line %s.  Argument %d is scalar.  This argument is array-valued.
1617 Function "%s" is also referenced at line %s.  The function result type and rank differ for the two references.
1618 Function "%s" is referenced at line %s.  The function result type and rank differ from this definition.
1619 Procedure "%s" is also referenced at line %s.  Argument %d is array-valued.  This argument is scalar.
1620 Program unit "%s" is referenced as a %s at line %s, but is defined or referenced as a %s here.
1621 "%s" is also referenced at line %s.  Argument %d is a label in one reference but not the other. They should be the same.
1622  "%s" is referenced at line %s. Argument %d is a user label. The dummy argument must be an alternate return argument.
1623 "%s" is also defined at line %s.  Argument %d must be an alternate return argument in both definitions.
1624 "%s" is also defined at line %s.  The %s attribute is specified in only one of the definitions.  It should be in both.
1625 "%s" is also referenced at line %s.  The %s directive is specified in one location, but not the other.
1626 Omitting the field width for edit descriptor "%c" is an extension to the Fortran standard.
1627 Equivalence-objects that are substrings, must not have a length of zero.
1628 Omitting the field width for edit descriptor "%c" is an extension to the Fortran standard.
1629 The following letter(s) were specified in a previous IMPLICIT %s statement: "%s", so must not be specified again.
1630 Macro "%s" exceeds limit of %d characters.
1631 Unterminated character literal encountered in preprocessing directive.
1632 The symbolic constant "%s" must not be defined.
1633 The array subscript for dimension %d does not fall within the defined range.
1634 The substring start or end value are outside the defined range.
1635 Argument mismatch:  "%s" and "%s" must both be functions or both be subroutines and have the same number of arguments.
1636 %s: "explain %s-message number" gives more information about each message
1637 Command line option %s has been applied to array "%s".
1638 A WHERE assignment statement that is a defined assignment must be elemental.
1639 "%s" is an elemental procedure.  It must not be used as an actual argument.
1640 The Fortran standard does not allow the END statement to be continued.
1641 The standard states that ALLOCATABLE, POINTER or SAVE must be specified for "%s" as it is default initialized type "%s".
1642 Procedure "%s" is being passed to PURE procedure "%s".  It must also be PURE.
1643 The %s enum has been changed.
1644 Array "%s" must not be statically intialized to be specified with -O reshape.
1645 Common block "/%s/" cannot be specified in both a !$OMP THREADPRIVATE directive and a !*$* SECTION_GP directive.
1646 Rank of the actual argument "%s" is less than the rank of the dummy argument "%s".
1647 "%s" is a CRI Pointee.  It will not be default initialized.
1648 The data-stmt-constant must be a named or literal constant or NULL() or a structure-constructor.
1649 A divisor of zero was detected in an expression.
1650 This argument cannot be a constant.
1651 %s variable %s not SHARED in enclosing parallel region.
1652 Routine %s was not inlined because it contains assignment of integer to a CRI pointer.
1653 The message severity for message number %d must not be changed to %s.  The change in severity will be ignored.
1654 Generic interface "%s" must not be specified with the %s directive, because the directive has the GLOBAL clause.
1655 The "%s" intrinsic must not be specified with the %s directive.  The intrinsic is not overloaded.
1656 A compiler directive may not follow a continued Fortran source line.
1657 Routine %s was not inlined because it is an ELEMENTAL procedure.
1658 Module output must be going to a .mod file, if the -J option is specified with a path name for module output.
1659 "-col120" will be translated to -extend_source which allows 132 columns.
1660 Procedure "%s" is defined at line %s with dummy argument "%s".  This argument is a procedure.
1661 "%s" is referenced at line %s and defined here. Argument "%s" is a dummy argument.  The actual argument is a procedure.
1662 The slash form of data initialization on a type declaration statement is nonstandard.
1663 The slash form of data initialization must not be used when the PARAMETER attribute is specified.
1664 The argument must be integer and large enough to hold an address.
1665 Module or procedure "%s" cannot be written out, because the compiler cannot open file "%s".
1666 Label variable "%s" must be scalar and of type Integer (kind=8).
1667 External function "%s" is referenced in a bounds specification expression.  It must not be recursive.
1668 External function "%s" is referenced in a bounds specification expression.  It must not have procedure arguments.
1669 "%s" is not supported on this platform.
1670 Macro "%s" redefined without intervening undef directive
