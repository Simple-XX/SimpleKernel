#ifndef _EFI_CONEX_H
#define _EFI_CONEX_H

/*++

Copyright (c) 2020 Kagurazaka Kotori <kagurazakakotori@gmail.com>

Module Name:

    eficonex.h

Abstract:

    EFI console extension protocols

--*/

//
// Simple Text Input Ex Protocol
//

#define EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL_GUID \
    { 0xdd9e7534, 0x7762, 0x4698, {0x8c, 0x14, 0xf5, 0x85, 0x17, 0xa6, 0x25, 0xaa} }

INTERFACE_DECL(_EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL);

typedef UINT8 EFI_KEY_TOGGLE_STATE;

typedef struct EFI_KEY_STATE {
    UINT32                                         KeyShiftState;
    EFI_KEY_TOGGLE_STATE                           KeyToggleState;
} EFI_KEY_STATE;

typedef struct {
    EFI_INPUT_KEY                                  Key;
    EFI_KEY_STATE                                  KeyState;
} EFI_KEY_DATA;

// Shift states
#define EFI_SHIFT_STATE_VALID                      0x80000000
#define EFI_RIGHT_SHIFT_PRESSED                    0x00000001
#define EFI_LEFT_SHIFT_PRESSED                     0x00000002
#define EFI_RIGHT_CONTROL_PRESSED                  0x00000004
#define EFI_LEFT_CONTROL_PRESSED                   0x00000008
#define EFI_RIGHT_ALT_PRESSED                      0x00000010
#define EFI_LEFT_ALT_PRESSED                       0x00000020
#define EFI_RIGHT_LOGO_PRESSED                     0x00000040
#define EFI_LEFT_LOGO_PRESSED                      0x00000080
#define EFI_MENU_KEY_PRESSED                       0x00000100
#define EFI_SYS_REQ_PRESSED                        0x00000200

// Toggle states
#define EFI_TOGGLE_STATE_VALID                     0x80
#define EFI_KEY_STATE_EXPOSED                      0x40
#define EFI_SCROLL_LOCK_ACTIVE                     0x01
#define EFI_NUM_LOCK_ACTIVE                        0x02
#define EFI_CAPS_LOCK_ACTIVE                       0x04

typedef
EFI_STATUS
(EFIAPI *EFI_INPUT_RESET_EX) (
    IN struct _EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL   *This,
    IN BOOLEAN                                     ExtendedVerification
    );

typedef
EFI_STATUS
(EFIAPI *EFI_INPUT_READ_KEY_EX) (
    IN struct _EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL   *This,
    OUT EFI_KEY_DATA                               *KeyData
    );

typedef
EFI_STATUS
(EFIAPI *EFI_SET_STATE) (
    IN struct _EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL   *This,
    IN EFI_KEY_TOGGLE_STATE                        *KeyToggleState
    );

typedef
EFI_STATUS
(EFIAPI *EFI_KEY_NOTIFY_FUNCTION) (
    IN EFI_KEY_DATA                                *KeyData
    );

typedef
EFI_STATUS
(EFIAPI *EFI_REGISTER_KEYSTROKE_NOTIFY) (
    IN struct _EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL   *This,
    IN EFI_KEY_DATA                                *KeyData,
    IN EFI_KEY_NOTIFY_FUNCTION                     KeyNotificationFunction,
    OUT VOID                                       **NotifyHandle
    );

typedef
EFI_STATUS
(EFIAPI *EFI_UNREGISTER_KEYSTROKE_NOTIFY) (
    IN struct _EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL   *This,
    IN VOID                                        *NotificationHandle
    );

typedef struct _EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL{
    EFI_INPUT_RESET_EX                             Reset;
    EFI_INPUT_READ_KEY_EX                          ReadKeyStrokeEx;
    EFI_EVENT                                      WaitForKeyEx;
    EFI_SET_STATE                                  SetState;
    EFI_REGISTER_KEYSTROKE_NOTIFY                  RegisterKeyNotify;
    EFI_UNREGISTER_KEYSTROKE_NOTIFY                UnregisterKeyNotify;
} EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL;

#endif
