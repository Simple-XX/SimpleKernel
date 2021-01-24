
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://github.com/bztsrc/raspi3-tutorial
// mailbox.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "mailbox.h"
#include "hardware.h"

mailbox_mail_t MAILBOX::call(mailbox_mail_t mail) {
    set(mail);
    mailbox_mail_t res = get(mail.channel);
    return res;
}

mailbox_mail_t MAILBOX::get(uint8_t channel) {
    mailbox_mail_t   res;
    mailbox_status_t stat;
    while (1) {
        // 等待有数据传回
        do {
            stat.empty = *((uint32_t *)MAILBOX_STATUS) & MAILBOX_EMPTY;
        } while (stat.empty);
        res.channel = *(uint32_t *)MAILBOX_READ & 0xF;
        if (res.channel == channel) {
            // 等到了需要的数据
            break;
        }
    }
    res.data = *(uint32_t *)MAILBOX_READ & 0xFFFFFFF0;
    return res;
}

// TODO
mailbox_mail_t MAILBOX::peak(uint8_t channel) {
    mailbox_mail_t res;
    res.data = 0;
    channel  = channel;
    return res;
}

void MAILBOX::set(mailbox_mail_t data) {
    mailbox_status_t stat;
    do {
        stat.empty = *((uint32_t *)MAILBOX_STATUS) & MAILBOX_EMPTY;
        stat.full  = *((uint32_t *)MAILBOX_STATUS) & MAILBOX_FULL;
    } while (stat.full);
    *((uint32_t *)MAILBOX_WRITE) = data.channel | data.data;
    return;
}

// TODO
mailbox_status_t MAILBOX::get_status(void) {
    mailbox_status_t res;
    res.empty = 1;
    return res;
}

// TODO
void MAILBOX::set_status(mailbox_status_t status) {
    status = status;
    return;
}

// TODO
mailbox_config_t MAILBOX::get_config(void) {
    mailbox_config_t res;
    return res;
}

// TODO
void MAILBOX::set_config(mailbox_config_t config) {
    config = config;
    return;
}
