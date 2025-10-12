% Тестовый файл Prolog с полным покрытием грамматики
% Unicode комментарии: こんにちは世界! مرحبا بالعالم!

/* Блочный комментарий
   Многострочный
   С кириллицей: Привет!
*/

% Факты
parent(tom, bob).
parent(tom, liz).
parent(bob, ann).
parent(bob, pat).
parent(pat, jim).

% Правила с различными атомами
grandparent(X, Z) :- parent(X, Y), parent(Y, Z).

% Кавычки и строки
greeting('Hello, World!').
message("Это строка в двойных кавычках").
unicode_atom('Привет мир').
escape_test('Line1\nLine2\tTab\'Quote\'').

% Числа
number_test(42).
negative_test(-123).
float_test(3.14159).
scientific(2.5e-10).

% Переменные
test_vars(X, Y, Z, _Anonymous).

% Списки
list_empty([]).
list_simple([1, 2, 3]).
list_nested([a, [b, c], [d, [e, f]]]).
list_vars([H|T]).

% Унификация
unify(X, X).

% Сложные термы
complex(foo(bar, baz)).
nested(f(g(h(x)))).
mixed(term(123, 'atom', [1,2,3], variable)).

% Операторы
arithmetic(X) :- X is 2 + 3 * 4.
comparison(X, Y) :- X > Y, X \= Y.

% Запросы
?- parent(tom, Who).
?- grandparent(tom, GrandChild).

