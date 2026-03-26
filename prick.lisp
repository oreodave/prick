;;; prick.lisp - 2026-03-26

;; Copyright (C) 2026 Aryadev Chavali

;; This program is distributed in the hope that it will be useful, but WITHOUT
;; ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
;; FOR A PARTICULAR PURPOSE.  See the Unlicense for details.

;; You may distribute and modify this code under the terms of the Unlicense,
;; which you should have received a copy of along with this program.  If not,
;; please go to <https://unlicense.org/>.

;;; Commentary:

;; A set of useful functions, macros, and types that I've implemented enough
;; times to require their own prick library.  There are a couple ways you can
;; use this file:
;; 1) Copy file and load it in your main.lisp.  Ensure your code is in a
;;    separate package for namespacing purposes.
;; 2) Copy file, move `defpackage' form into your packages.lisp, and add this
;;    file as a component in your ASDF system definition.

;;; Code:

(defpackage #:prick
  (:use :cl)
  (:export
   ;; Threading macros
   :--> :->> :-<>
   ;; Anonymous function constructors utilising threading macros
   :$-> :$>> :$<>
   ;; Strictly typed functions and function calling
   :-> :fn :call-rev
   ;; General purpose functions
   :range :split :remove-at-indices :rev-map))

(in-package #:prick)

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
                :for func :in (cdr forms)
                :collect (list placeholder (--transform-symbols-to-unary func)) :into xs
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
        :do (setq acc (append (--transform-symbols-to-unary func) (list acc)))
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
        :for canon-func := (--transform-symbols-to-unary func)
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

(fn range (&key (start 0) (end 0) (step 1))
    (-> (&key (:start fixnum) (:end fixnum) (:step fixnum)) list)
  "Return list of integers in interval [`start', `end').  If `step' is not 1,
then each member is `step' distance apart i.e. {`start' + (n * `step') | n from 0
till END}.

If END is not given, return interval [0, START)."
  (declare (type integer start end step))
  (if (< end start)
      (error (format nil "~a < ~a" end start))
      (loop :for i :from start :to (1- end) :by step
            :collect i)))

(fn split (n lst) (-> (fixnum sequence) (values sequence sequence))
  "Return two sequences of `lst': lst[0..`n'] and lst[`n'..]."
  (values (subseq lst 0 n)
          (subseq lst n)))

(fn remove-at-indices (indices lst) (-> (list sequence) list)
  "Return `lst' with all items at an index specified in `indices' removed.

i.e. (remove-at-indices indices (l-1...l-m)) => (l-x where x is not in indices)."
  (loop :for i :from 0 :to (1- (length lst))
        :for item :in (coerce lst 'list)
        :if (not (member i indices))
          :collect item))

(fn rev-map (indicator lst &key (key-eq #'eq))
    (-> (function list &key (:key-eq function)) list)
  "Given some sequence of elements `lst' and a function `indicator': `lst' -> A for
some set A, return the reverse mapping of `indicator' on `lst'

i.e. Return `indicator'^-1: A -> {`lst'}.

`key-eq' is used for testing if any two elements of A are equivalent."
  (declare (type (function (t) t) indicator)
           (type sequence lst)
           (type (function (t t) boolean) key-eq))
  (loop :with assoc-list := nil
        :for element :in (coerce lst 'list)
        :for key := (funcall indicator element)
        :if (assoc key assoc-list :test key-eq)
          :do (push element (cdr (assoc key assoc-list :test key-eq)))
        :else
          :do (setq assoc-list (cons (list key element) assoc-list))
        :finally (return assoc-list)))
