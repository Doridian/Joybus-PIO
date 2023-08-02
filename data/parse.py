def crc_poly(data, n, poly, crc=0):
    g = 1 << n | poly  # Generator polynomial

    # Loop over the data
    for d in data:
        # XOR the top byte in the CRC with the input byte
        crc ^= d << (n - 8)

        # Loop over all the bits in the byte
        for _ in range(8):
            # Start by shifting the CRC, so we can check for the top bit
            crc <<= 1

            # XOR the CRC if the top bit is 1
            if crc & (1 << n):
                crc ^= g

    # Return the CRC value
    return crc

#  It uses a standard CRC8 function, with a seed (initial value) of 0x00, and 0x85 for the polynomial.
def n64_checksum(data):
    real_data = data[:32]
    checksum = data[32]
    calc_checksum = crc_poly(real_data, 8, 0x85, 0x00)
    if checksum != calc_checksum:
        print(checksum, calc_checksum)
        raise ValueError("INVALID")
    return real_data

def n64_add_checksum(real_data):
    calc_checksum = crc_poly(real_data, 8, 0x85, 0x00)
    return real_data + [calc_checksum]

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

print(n64_add_checksum([0x55, 0xAA]*16))
