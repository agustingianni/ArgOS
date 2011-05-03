#ifndef GATES_H_
#define GATES_H_

 void set_trap_gate(int n, void *handler, int dpl);
 void set_interrupt_gate(int n, void *handler, int dpl);
 void set_task_gate(int n, void *handler, int dpl);

#endif /*GATES_H_*/
