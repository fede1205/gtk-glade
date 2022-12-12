#include <gtk/gtk.h>
#include <gtk-3.0/gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <PJ_RPI.h>
#include "PJ_RPI.h"

struct bcm2835_peripheral gpio = {GPIO_BASE};
struct bcm2835_peripheral bsc0 = {BSC0_BASE};

gpointer lblptr;

int map_peripheral(struct bcm2835_peripheral *p)
{
    if ((p->mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0)
    {
        printf("Failed to open /dev/mem, try checking permissions.\n");
        return -1;
    }

    p->map = mmap(
        NULL,
        BLOCK_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        p->mem_fd,
        p->addr_p);

    if (p->map == MAP_FAILED)
    {
        perror("mmap");
        return -1;
    }

    p->addr = (volatile unsigned int *)p->map;

    return 0;
}

void unmap_peripheral(struct bcm2835_peripheral *p)
{

    munmap(p->map, BLOCK_SIZE);
    close(p->mem_fd);
}

void dump_bsc_status()
{

    unsigned int s = BSC0_S;

    printf("BSC0_S: ERR=%d  RXF=%d  TXE=%d  RXD=%d  TXD=%d  RXR=%d  TXW=%d  DONE=%d  TA=%d\n",
           (s & BSC_S_ERR) != 0,
           (s & BSC_S_RXF) != 0,
           (s & BSC_S_TXE) != 0,
           (s & BSC_S_RXD) != 0,
           (s & BSC_S_TXD) != 0,
           (s & BSC_S_RXR) != 0,
           (s & BSC_S_TXW) != 0,
           (s & BSC_S_DONE) != 0,
           (s & BSC_S_TA) != 0);
}

void wait_i2c_done()
{
    int timeout = 50;
    while ((!((BSC0_S)&BSC_S_DONE)) && --timeout)
    {
        usleep(1000);
    }
    if (timeout == 0)
        printf("wait_i2c_done() timeout. Something went wrong.\n");
}

void i2c_init()
{
    INP_GPIO(0);
    SET_GPIO_ALT(0, 0);
    INP_GPIO(1);
    SET_GPIO_ALT(1, 0);
}

int SetProgramPriority(int priorityLevel)
{
    struct sched_param sched;

    memset(&sched, 0, sizeof(sched));

    if (priorityLevel > sched_get_priority_max(SCHED_RR))
        priorityLevel = sched_get_priority_max(SCHED_RR);

    sched.sched_priority = priorityLevel;

    return sched_setscheduler(0, SCHED_RR, &sched);
}

void end_program(GtkWidget *wid, gpointer ptr)
{
    gtk_main_quit();
}
void input1(GtkWidget *wid, gpointer ptr)
{
    char Pin1State[50];
    if (GPIO_READ(17))
    {
        printf("GPIO 17 high \n");
        sprintf(Pin1State, "Input GPIO 17: high");
    }
    else
    {
        printf("GPIO 17 is low\n");
        sprintf(Pin1State, "Input GPIO 17: low");
    }
    gtk_label_set_text(GTK_LABEL(ptr), Pin1State);
}

void input2(GtkWidget *wid, gpointer ptr)
{
    char Pin2State[50];
    if (GPIO_READ(22))
    {
        printf("GPIO 22 high \n");
        sprintf(Pin2State, "Input GPIO 22: high");
    }
    else
    {
        printf("GPIO 22 is low\n");
        sprintf(Pin2State, "Input GPIO 22: low");
    }
    gtk_label_set_text(GTK_LABEL(ptr), Pin2State);
}

void output1(GtkWidget *wid, gpointer ptr)
{
    INP_GPIO(26);
    OUT_GPIO(26);   
if (GPIO_READ(26)){
   GPIO_CLR = 1 << 26;
}
else {
    GPIO_SET = 1 << 26;
}
 fflush(stdout);
}

void output2(GtkWidget *wid, gpointer ptr)
{
    INP_GPIO(20);
    OUT_GPIO(20);   
if (GPIO_READ(20)){
   GPIO_CLR = 1 << 20;
}
else {
    GPIO_SET = 1 << 20;
}
 fflush(stdout);
}

void combo_changed (GtkWidget *wid, gpointer ptr)
{
  int sel = gtk_combo_box_get_active (GTK_COMBO_BOX (wid));
  int gpio_pin = sel;
  // read seected pin
  char Pin1State[50];
    if (GPIO_READ(gpio_pin))
    {
        sprintf(Pin1State, "Input GPIO %d high", gpio_pin);
    }
    else
    {
        sprintf(Pin1State, "Input GPIO %d: low", gpio_pin);
    }
    gtk_label_set_text(GTK_LABEL(lblptr), Pin1State);

  char *selected = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (wid));
  printf ("The value of the combo is %d %s\n", sel, selected);
}

void input_combo(GtkWidget *wid, gpointer ptr)
{
  int sel = gtk_combo_box_get_active (GTK_COMBO_BOX (ptr));
  char Pin1State[50];
    if (GPIO_READ(sel))
    {
        sprintf(Pin1State, "Input GPIO %d: high", sel);
    }
    else
    {
        sprintf(Pin1State, "Input GPIO %d: low", sel);
    }
    gtk_label_set_text(GTK_LABEL(lblptr), Pin1State);
  char *selected = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (wid));
  printf ("The value of the combo is %d %s\n", sel, selected);
}



void main(int argc, char *argv[])
{
    
    if (map_peripheral(&gpio) == -1)
    {
        printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
        //return -1;
    }

    gtk_init(&argc, &argv);
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *btn = gtk_button_new_with_label("Close window");
    g_signal_connect(btn, "clicked", G_CALLBACK(end_program), NULL);
    g_signal_connect(win, "delete_event", G_CALLBACK(end_program), NULL);
    GtkWidget *lbl1 = gtk_label_new("Input GPIO 17:");
    GtkWidget *lbl2 = gtk_label_new("Input GPIO 22:");
    GtkWidget *lbl3 = gtk_label_new("Input selected:");
    lblptr = lbl3;
    GtkWidget *btn1 = gtk_button_new_with_label("Controleer status van GPIO 17");
    GtkWidget *btn2 = gtk_button_new_with_label("Controleer status van GPIO 22");
    GtkWidget *btn3 = gtk_button_new_with_label("controleer status selected");
    GtkWidget *chk1 = gtk_check_button_new_with_label("Output gpio 26");
    GtkWidget *chk2 = gtk_check_button_new_with_label("Output gpio 20");
    GtkWidget *comb = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(comb),"gpio 0");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(comb),"gpio 1");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(comb),"gpio 2");
    g_signal_connect(btn1, "clicked", G_CALLBACK(input1), lbl1);
    g_signal_connect(btn2, "clicked", G_CALLBACK(input2), lbl2);
    g_signal_connect(chk1, "toggled", G_CALLBACK(output1), NULL);
    g_signal_connect(chk2, "toggled", G_CALLBACK(output2), NULL);
    //g_signal_connect (comb, "changed", G_CALLBACK (combo_changed), NULL);
    g_signal_connect(btn3, "clicked", G_CALLBACK(input_combo), comb);
    GtkWidget *box = gtk_vbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), btn1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), lbl1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), btn2, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), lbl2, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), chk1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), chk2, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), comb, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), btn3, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), lbl3, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), btn, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(win), box);
    gtk_widget_show_all(win);
    gtk_main();
}