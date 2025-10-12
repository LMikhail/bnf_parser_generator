; Тестовый файл Clojure с полным покрытием грамматики
; Unicode комментарии: こんにちは世界! مرحبا بالعالم! Привет мир!

;; Namespace
(ns bnf-parser-test
  (:require [clojure.string :as str]))

;; Числа - все варианты
42
-123
3.14159
2.5e-10
0xFF
0b1010
0o755
22/7

;; Строки с Unicode и экранированием
"Hello, World!"
"Привет мир! 你好世界!"
"Line1\nLine2\tTabbed"
"Quote:\"test\" Backslash:\\"

;; Символы
:keyword
:namespaced/keyword
::auto-resolved
'symbol
'namespaced/symbol

;; Character literals
\a
\newline
\space
\tab
\u0041
\😀

;; Булевы и nil
true
false
nil

;; Списки и структуры данных
'()
'(1 2 3)
'(a b c (nested list))

;; Векторы
[]
[1 2 3]
[a b [nested vector]]

;; Мапы
{}
{:name "Test" :version 1}
{:unicode "Привет" :nested {:value 42}}

;; Сеты
#{}
#{1 2 3}
#{:a :b :c}

;; Функции
(defn factorial [n]
  (if (<= n 1)
    1
    (* n (factorial (dec n)))))

(defn greet [name]
  (str "Hello, " name "!"))

;; Let bindings
(let [x 10
      y 20
      z (+ x y)]
  z)

;; Lambda
(fn [x] (* x x))
#(* % %)

;; Макросы
(defmacro unless [test then]
  `(if (not ~test) ~then))

;; Threading макросы
(-> 5
    (+ 3)
    (* 2)
    (- 1))

(->> [1 2 3 4 5]
     (map #(* % 2))
     (filter even?)
     (reduce +))

;; Metadata
^:private (defn secret [] :shh)
^{:doc "Test function"} (defn test-fn [] nil)

;; Reader macros
#'symbol  ; var quote
@atom-val ; deref
#(+ % 1)  ; anonymous function
#"regex"  ; regex pattern

;; Destructuring
(let [{:keys [a b c]} {:a 1 :b 2 :c 3}]
  (+ a b c))

(let [[x y & rest] [1 2 3 4 5]]
  [x y rest])

;; Многострочные выражения
(defn complex-function
  "Unicode docstring: Привет!"
  [x y z]
  (let [sum (+ x y z)
        product (* x y z)]
    {:sum sum
     :product product
     :average (/ sum 3.0)}))

;; Вызовы
(factorial 5)
(greet "Мир")
(println "Тест завершен!")

