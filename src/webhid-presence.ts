export const LINERACK_DEVELOPMENT_VENDOR_ID = 0xcafe;
export const LINERACK_DEVELOPMENT_PRODUCT_ID = 0x4c52;

export const LINERACK_HID_FILTER: HIDDeviceFilter = {
  vendorId: LINERACK_DEVELOPMENT_VENDOR_ID,
  productId: LINERACK_DEVELOPMENT_PRODUCT_ID,
};

export const isLineRackHidDevice = (device: HIDDevice): boolean =>
  device.vendorId === LINERACK_DEVELOPMENT_VENDOR_ID &&
  device.productId === LINERACK_DEVELOPMENT_PRODUCT_ID;

export const findAuthorizedLineRackDevice = async (hid: HID): Promise<HIDDevice | undefined> =>
  (await hid.getDevices()).find(isLineRackHidDevice);

export const requestLineRackDevice = async (hid: HID): Promise<HIDDevice | undefined> =>
  (await hid.requestDevice({ filters: [LINERACK_HID_FILTER] }))[0];
