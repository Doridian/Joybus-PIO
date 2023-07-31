def reflect_data(x, width):
    # See: https://stackoverflow.com/a/20918545
    if width == 8:
        x = ((x & 0x55) << 1) | ((x & 0xAA) >> 1)
        x = ((x & 0x33) << 2) | ((x & 0xCC) >> 2)
        x = ((x & 0x0F) << 4) | ((x & 0xF0) >> 4)
    elif width == 16:
        x = ((x & 0x5555) << 1) | ((x & 0xAAAA) >> 1)
        x = ((x & 0x3333) << 2) | ((x & 0xCCCC) >> 2)
        x = ((x & 0x0F0F) << 4) | ((x & 0xF0F0) >> 4)
        x = ((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8)
    elif width == 32:
        x = ((x & 0x55555555) << 1) | ((x & 0xAAAAAAAA) >> 1)
        x = ((x & 0x33333333) << 2) | ((x & 0xCCCCCCCC) >> 2)
        x = ((x & 0x0F0F0F0F) << 4) | ((x & 0xF0F0F0F0) >> 4)
        x = ((x & 0x00FF00FF) << 8) | ((x & 0xFF00FF00) >> 8)
        x = ((x & 0x0000FFFF) << 16) | ((x & 0xFFFF0000) >> 16)
    else:
        raise ValueError('Unsupported width')
    return x

def crc_poly(data, n, poly, crc=0, ref_in=False, ref_out=False, xor_out=0):
    g = 1 << n | poly  # Generator polynomial

    # Loop over the data
    for d in data:
        # Reverse the input byte if the flag is true
        if ref_in:
            d = reflect_data(d, 8)

        # XOR the top byte in the CRC with the input byte
        crc ^= d << (n - 8)

        # Loop over all the bits in the byte
        for _ in range(8):
            # Start by shifting the CRC, so we can check for the top bit
            crc <<= 1

            # XOR the CRC if the top bit is 1
            if crc & (1 << n):
                crc ^= g

    # Reverse the output if the flag is true
    if ref_out:
        crc = reflect_data(crc, n)

    # Return the CRC value
    return crc ^ xor_out

#  It uses a standard CRC8 function, with a seed (initial value) of 0x00, and 0x85 for the polynomial.
def n64_checksum(data):
    real_data = data[:32]
    checksum = data[32]
    calc_checksum = crc_poly(real_data, 8, 0x85, 0x00)
    if checksum != calc_checksum:
        print(checksum, calc_checksum)
        raise ValueError("INVALID")
    return real_data

data = {}
max_addr = 0
with open("mpakdata.txt", "r") as f:
    for l in f.readlines():
        ls = l.split("|")
        cmd = ls[0].strip().lower()
        if  cmd != "reading...":
            continue
        addr = int(ls[1].strip().lower(), 16)
        payload = ls[3]

        if addr > max_addr:
            max_addr = addr

        payload_arr = []
        payload_bytes = payload.strip().split(' ')
        for b in payload_bytes:
            payload_arr.append(int(b.strip().lower(), 16))

        payload_arr_filtered = n64_checksum(payload_arr)
        if payload_arr_filtered:
            data[addr] = bytearray(payload_arr_filtered)
    
with open("mpakdata.raw", "wb") as f:
    for i in range(max_addr+1):
        if i not in data:
            raise ValueError(f"Missing {i}")
        f.write(data[i])
