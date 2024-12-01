int main(void) {
    put_str("Welcome,\nI am kernel!\n");
    init_all();

//    thread_start("k_thread_a",31,k_thread_a,"argA");
//    thread_start("k_thread_b",8,k_thread_b,"argB ");

    intr_enable();

//    while(1){
//       //intr_disable();
//       console_put_str("Main ");
//       //intr_enable();
//    }
    while (1);
    return 0;
}