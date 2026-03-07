;;; prick_functions.lisp - 2026-03-07

;; Copyright (C) 2026 Aryadev Chavali

;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the Unlicense
;; for details.

;; You may distribute and modify this code under the terms of the
;; Unlicense, which you should have received a copy of along with this
;; program.  If not, please go to <https://unlicense.org/>.

;;; Commentary:

;; A set of useful functions that I've designed for use in Common Lisp.  There
;; are a couple ways you may utilise this file:
;; 1) Copy file and load it in your main.lisp.  Ensure your code is in a
;;    separate package for namespacing purposes.
;; 2) Copy file, move `defpackage' form into your packages.lisp, and add this
;;    file as a component in your ASDF system definition.

;;; Code:

(defpackage #:prick.functions
  (:use :cl)
  (:export
   :range :split :remove-at-indices :rev-map))

(in-package #:prick.functions)

(defun range (&key (start 0) (end 0) (step 1))
  "Return list of integers in interval [`start', `end').  If `step' is not 1,
then each member is `step' distance apart i.e. {`start' + (n * `step') | n from 0
till END}.

If END is not given, return interval [0, START)."
  (declare (type integer start end step))
  (if (< end start)
      (error (format nil "~a < ~a" end start))
      (loop :for i :from start :to (1- end) :by step
            :collect i)))

(defun split (n lst)
  "Return two sequences of `lst': lst[0..`n'] and lst[`n'..]."
  (declare (type integer n)
           (type sequence lst))
  (values (subseq lst 0 n)
          (subseq lst n)))

(defun remove-at-indices (indices lst)
  "Return `lst' with all items at an index specified in `indices' removed.

i.e. (remove-at-indices indices (l-1...l-m)) => (l_x where x is not in indices)."
  (declare (type list indices)
           (type lst sequence))
  (loop :for i :from 0 :to (1- (length lst))
        :for item :in (coerce lst 'list)
        :if (not (member i indices))
          :collect item))

(defun rev-map (indicator lst &key (key-eq #'eq))
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
