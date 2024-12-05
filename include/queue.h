#include <stdio.h>
#include <stdlib.h>

// 定义队列元素的结构体
typedef struct Node {
    int data;              // 存储队列中的数据
    struct Node* next;     // 指向下一个节点的指针
} Node;

// 定义队列结构体
typedef struct {
    Node* front;           // 队列的头
    Node* rear;            // 队列的尾
} queue_t;

// 队列初始化
void queue_init(queue_t* q) {
    q->front = q->rear = NULL;  // 初始化时，队列为空
}

// 判断队列是否为空
int queue_empty(queue_t* q) {
    return q->front == NULL;   // 如果队列的头为空，则队列为空
}

// 增加元素到队列
void queue_add(queue_t* q, int id) {
    // 创建一个新的节点
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        printf("Memory allocation failed\n");
        return;
    }
    newNode->data = id;
    newNode->next = NULL;

    if (q->rear == NULL) {
        // 如果队列为空，新节点既是头又是尾
        q->front = q->rear = newNode;
    } else {
        // 将新节点添加到队尾
        q->rear->next = newNode;
        q->rear = newNode;
    }
}

// 删除队列中的元素
int queue_remove(queue_t* q) {
    if (queue_empty(q)) {
        printf("Queue is empty, nothing to remove\n");
        return -1;  // 返回 -1 表示队列为空，无法删除
    }

    // 从队列头部删除元素
    Node* temp = q->front;
    int data = temp->data;
    
    q->front = q->front->next;

    if (q->front == NULL) {
        // 如果删除后队列为空，尾指针也需要置空
        q->rear = NULL;
    }

    free(temp);  // 释放删除的节点内存
    return data;
}