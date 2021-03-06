@ RUN: llvm-mc -triple armv7-elf -filetype obj -o - %s | llvm-readobj -t \
@ RUN:    | FileCheck %s

	.syntax unified

	.thumb

	.type implicit_function,%function
implicit_function:
	nop

	.type implicit_data,%object
implicit_data:
	.long 0

	.arm
	.type arm_function,%function
arm_function:
	nop

	.thumb

	.text

untyped_text_label:
	nop

	.type explicit_function,%function
explicit_function:
	nop

	.data

untyped_data_label:
	nop

	.type explicit_data,%object
explicit_data:
	.long 0

@ CHECK: Symbol {
@ CHECK:   Name: arm_function
@ CHECK:   Value: 0x6
@ CHECK:   Type: Function
@ CHECK: }

@ CHECK: Symbol {
@ CHECK:   Name: explicit_data
@ CHECK:   Value: 0x2
@ CHECK:   Type: Object
@ CHECK: }

@ CHECK: Symbol {
@ CHECK:   Name: explicit_function
@ CHECK:   Value: 0xD
@ CHECK:   Type: Function
@ CHECK: }

@ CHECK: Symbol {
@ CHECK:   Name: implicit_data
@ CHECK:   Value: 0x2
@ CHECK:   Type: Object
@ CHECK: }

@ CHECK: Symbol {
@ CHECK:   Name: implicit_function
@ CHECK:   Value: 0x1
@ CHECK:   Type: Function
@ CHECK: }

@ CHECK: Symbol {
@ CHECK:   Name: untyped_data_label
@ CHECK:   Value: 0x0
@ CHECK:   Type: None
@ CHECK: }

@ CHECK: Symbol {
@ CHECK:   Name: untyped_text_label
@ CHECK:   Value: 0xA
@ CHECK:   Type: None
@ CHECK: }

