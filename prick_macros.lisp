;;; prick_macros.lisp - 2026-03-07

;; Copyright (C) 2026 Aryadev Chavali

;; This program is distributed in the hope that it will be useful, but WITHOUT
;; ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
;; FOR A PARTICULAR PURPOSE.  See the Unlicense for details.

;; You may distribute and modify this code under the terms of the Unlicense,
;; which you should have received a copy of along with this program.  If not,
;; please go to <https://unlicense.org/>.

;;; Commentary:

;; A set of useful macros I've designed for use in Common Lisp.  There are a
;; couple ways you may utilise this file:
;; 1) Copy file and load it in your main.lisp.  Ensure your code is in a
;;    separate package for namespacing purposes.
;; 2) Copy file, move `defpackage' form into your packages.lisp, and add this
;;    file as a component in your ASDF system definition.

;;; Code:

(defpackage #:prick.macros
  (:use :cl)
  (:export
   ;; Threading macros
   :--> :->> :-<>
   ;; Anonymous function constructors utilising threading macros
   :$-> :$>> :$<>
   ;; Strictly typed functions and function calling
   :-> :fn :call-rev))

(in-package #:prick-macros)

(defun --transform-symbols-to-unary (form)
  (if (symbolp form)
      (list form)
      form))

(defmacro --> (placeholder &body forms)
  "Fold `forms' recursively such that, given consecutive forms, the first form is
lexically bound to `placeholder' for the second form.  Evaluate the form
generated after folding has completed.

(--> x (a1 a2...) (b1 b2...) (c1 c2...)) =>
(let ((x (a1 a2 ...)))
  (let ((x (b1 b2 ...)))
    (let ((x (c1 c2 ...)))
      x)))

Includes transformer where symbols (after the first form) are considered
unary functions i.e.
(--> x a b c) =>
(let ((x a))
  (let ((x (b x)))
    (let ((x (c x)))
      x)))"
  `(let* ,(loop :for i :from 1
                :for f :in (cdr forms)
                :for canon-f := (if (symbolp f)
                                    (list f placeholder)
                                    f)
                :collect `(,placeholder ,canon-f) :into xs
                :finally (return (cons `(,placeholder ,(car forms)) xs)))
     ,placeholder))

(defmacro ->> (&rest forms)
  "Fold FORMS recursively such that, given consecutive forms, the first form
becomes the last argument of the second form.  Evaluate the form generated after
folding has completed.

(->> (a1 ... al) (b1 ... bm) (c1 ... cm)) => (c1 ... cm (b1 ... bn (a1 ... al)))

Includes transformer where symbols (after the first form) are considered unary
functions i.e. (->> a b c) => (c (b a))"
  (loop :with acc := (car forms)
        :for func :in (cdr forms)
        :for canon-func := (--transform-symbols-to-unary func)
        :do (setq acc (append canon-func (list acc)))
        :finally (return acc)))

(defmacro -<> (&rest forms)
  "Fold FORMS recursively such that, given consecutive forms, the first form
becomes the first argument of the second form.  Evaluate the form generated
after folding has completed.

(-<> (a1 ... al) (b1 ... bm) (c1 ... cn)) => (c1 (b1 (a1 ... al) ... bm) ... cn)

Includes transformer where symbols (after the first form) are considered unary
functions i.e. (-<> a b c) => (c (b a))"
  (loop :with acc = (car forms)
        :for func :in (cdr forms)
        :for canon-func := (if (symbolp func) (list func) func)
        :do (push acc (cdr canon-func))
        :do (setq acc canon-func)
        :finally (return acc)))

(defmacro $-> (capture &rest forms)
  "Return an anonymous unary function (with argument named `capture') that feeds
its argument into a `-->' chain composed of `forms'.  Note that `capture' is
also used as the placeholder value in said `-->' chain."
  `(lambda (,capture)
     (--> ,capture ,capture ,@forms)))

(defmacro $>> (&rest forms)
  "Return an anonymous unary function that feeds its argument into a `->>' chain
composed of `forms'."
  (let ((capture (gensym)))
    `(lambda (,capture)
       (->> ,capture ,@forms))))

(defmacro $<> (&rest forms)
  "Return an anonymous unary function that feeds its argument into a `-<>' chain
composed of `forms'."
  (let ((capture (gensym)))
    `(lambda (,capture)
       (-<> ,capture ,@forms))))

(deftype -> (args result)
  "Simple type alias for functions."
  `(function ,args ,result))

(defmacro fn (name lambda-list type &body body)
  "Construct a function `name' that takes arguments `lambda-list' with body
`body'.  `type' is used as the type of the function constructed via a declaim."
  `(progn
     (declaim (ftype ,type ,name))
     (defun ,name ,lambda-list
       ,@body)))

(defmacro call-rev (func-name &rest arguments)
  "Call function `func-name' with `arguments' reversed.

i.e. (call-rev f arg-1 ... arg-n) => (f arg-n ... arg-1).

Interacts well with the threading macro family (`-->', `->>', `-<>')"
  `(,func-name ,@(reverse arguments)))
