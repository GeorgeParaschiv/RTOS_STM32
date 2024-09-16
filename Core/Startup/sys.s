	.syntax unified
	.cpu cortex-m4
	.fpu softvfp
	.thumb

.global SVC_Handler
.global StartTask
.global current_stackptr
.global scheduled_stackptr
.global stack_p
.global PendSV_Handler_Main
.global PendSV_Handler

// SVC Handler
.thumb_func
PendSV_Handler:

	// Stores current tasks context into stack
	MRS R0, PSP
  	LDR R2, = current_stackptr
 	STMDB R0!, {R4-R11}
 	STR R0, [R2]

	BL PendSV_Handler_Main

 	// Starts next scheduled task
 	LDR R0, = scheduled_stackptr
 	LDR R0, [R0]
  	LDMIA R0!, {R4-R11}
	MSR PSP, R0
	MOV LR, #0xFFFFFFFD
 	BX LR

.thumb_func
SVC_Handler:

	// Stores SVC Call # Number into R1
  	TST LR, #4
  	ITE EQ
  	MRSEQ R0, MSP
  	MRSNE R0, PSP
  	LDR R1, [R0, #24]
  	LDRB R1, [R1, #-2]

  	// If SVC Call Number is 1 run Start Task
  	TST R1, #2
  	IT EQ
  	BEQ Start_Task

  	BL Context_Switch

  	BL SVC_Handler_Main

  	BL Start_Task

// Start Task
 .thumb_func
 Start_Task:
 	LDR R0, = scheduled_stackptr
 	LDR R0, [R0]
  	LDMIA R0!, {R4-R11}
	MSR PSP, R0
	MOV LR, #0xFFFFFFFD
 	BX LR

// Context Switch
 .thumb_func
 Context_Switch:
 	LDR R2, = current_stackptr
 	STMDB R0!, {R4-R11}
 	STR R0, [R2]
 	BX LR
