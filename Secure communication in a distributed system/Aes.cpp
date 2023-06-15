#include "Aes.h"


void RotWord(unsigned char* x) {
    unsigned char c = x[0];
    x[0] = x[1];
    x[1] = x[2];
    x[2] = x[3];
    x[3] = c;
}

void SubWord(unsigned char* x) {
    int i;
    for (i = 0; i < 4; i++) {
        x[i] = sbox[x[i] / 16][x[i] % 16];
    }
}

void xorW(unsigned char* z, unsigned char* y, unsigned char* x) {
    int i;
    for (i = 0; i < 4; i++) {
        z[i] = x[i] ^ y[i];
    }
}

unsigned char xtime(unsigned char x)
{
    if ((x >> 7) & 1)
        return (x << 1) ^ 0x1b;
    else
        return x << 1;
}


void Rcon(unsigned char* x, unsigned int n) {
    unsigned int i;
    unsigned char c = 1;
    for (i = 0; i < n - 1; i++) {
        c = xtime(c);
    }
    x[0] = c;
    x[1] = x[2] = x[3] = 0;
}
void KeyExpansion(unsigned char key[], unsigned char w[]) {
    unsigned char temp[4];
    unsigned char rcon[4];

    unsigned int i = 0;
    while (i < Nk) {
        w[4 * i] = key[4 * i];
        w[4 * i + 1] = key[4 * i + 1];
        w[4 * i + 2] = key[4 * i + 2];
        w[4 * i + 3] = key[4 * i + 3];
        i++;
    }
    i = Nk;
    while (i < Nb * (Nr + 1)) {
        temp[0] = w[4 * i - 4];
        temp[1] = w[4 * i - 3];
        temp[2] = w[4 * i - 2];
        temp[3] = w[4 * i - 1];
        /*cout << "temp\n";
        unsigned char h, l;
        for (int i = 0; i < 4; i++)
        {
            h = temp[i] / 16;
            l = temp[i] % 16;
            if (h <= 9)
                h = h + '0';
            else
                h = h + 'a' - 10;
            if (l <= 9)
                l = l + '0';
            else
                l = l + 'a' - 10;
            cout << hex << h;
            cout << hex << l;
        }
        cout << "\n";*/
        if (i % Nk == 0) {
            RotWord(temp);
            /*cout << "AfterRotWord\n";
            unsigned char h, l;
            for (int i = 0; i < 4; i++)
            {
                h = temp[i] / 16;
                l = temp[i] % 16;
                if (h <= 9)
                    h = h + '0';
                else
                    h = h + 'a' - 10;
                if (l <= 9)
                    l = l + '0';
                else
                    l = l + 'a' - 10;
                cout << hex << h;
                cout << hex << l;
            }
            cout << "\n";*/
            SubWord(temp);
            /*cout << "AfterSubWord\n";
            for (int i = 0; i < 4; i++)
            {
                h = temp[i] / 16;
                l = temp[i] % 16;
                if (h <= 9)
                    h = h + '0';
                else
                    h = h + 'a' - 10;
                if (l <= 9)
                    l = l + '0';
                else
                    l = l + 'a' - 10;
                cout << hex << h;
                cout << hex << l;
            }
            cout << "\n";*/
            Rcon(rcon, i / Nk);
            /*cout << "Rcon\n";
            for (int i = 0; i < 4; i++)
            {
                h = rcon[i] / 16;
                l = rcon[i] % 16;
                if (h <= 9)
                    h = h + '0';
                else
                    h = h + 'a' - 10;
                if (l <= 9)
                    l = l + '0';
                else
                    l = l + 'a' - 10;
                cout << hex << h;
                cout << hex << l;
            }
            cout << "\n";*/
            xorW(temp, temp, rcon);
            /*cout << "AfterXorRcon\n";
            for (int i = 0; i < 4; i++)
            {
                h = temp[i] / 16;
                l = temp[i] % 16;
                if (h <= 9)
                    h = h + '0';
                else
                    h = h + 'a' - 10;
                if (l <= 9)
                    l = l + '0';
                else
                    l = l + 'a' - 10;
                cout << hex << h;
                cout << hex << l;
            }
            cout << "\n";*/
        }
        else if (Nk > 6 && i % Nk == 4) {
            SubWord(temp);
            /*unsigned char h, l;
            cout << "AfterSubWord\n";
            for (int i = 0; i < 4; i++)
            {
                h = temp[i] / 16;
                l = temp[i] % 16;
                if (h <= 9)
                    h = h + '0';
                else
                    h = h + 'a' - 10;
                if (l <= 9)
                    l = l + '0';
                else
                    l = l + 'a' - 10;
                cout << hex << h;
                cout << hex << l;
            }
            cout << "\n";*/
        }

        w[4 * i] = w[4 * i - 4 * Nk] ^ temp[0];
        w[4 * i + 1] = w[4 * i + 1 - 4 * Nk] ^ temp[1];
        w[4 * i + 2] = w[4 * i + 2 - 4 * Nk] ^ temp[2];
        w[4 * i + 3] = w[4 * i + 3 - 4 * Nk] ^ temp[3];
        i++;
    }
}

void AddRoundKey(unsigned char state[], unsigned char* key) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < Nb; j++) {
            unsigned char c = state[i * 4 + j] ^ key[i * 4 + j];
            state[i * 4 + j] = c;
        }
    }
}

void SubBytes(unsigned char state[]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < Nb; j++) {
            state[4 * i + j] = sbox[state[4 * i + j] / 16][state[4 * i + j] % 16];
        }
    }
}

void ShiftRows(unsigned char state[]) {
    unsigned char tmp[16];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < Nb; j++) {
            tmp[4 * i + j] = state[4 * ((i + j) % 4) + j];
        }
    }
    memcpy(state, tmp, 16);
}

void MixColumns(unsigned char state[]) {
    unsigned char tmp[16];
    memset(tmp, 0, 16);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                int b = 1;
                unsigned char c = state[j * 4 + k], aux = 0;
                while (b <= M[i][k])
                {
                    if ((b & M[i][k]) != 0)
                    {
                        aux = aux ^ c;
                    }
                    c = xtime(c);
                    b = b << 1;
                }
                tmp[4 * j + i] = tmp[4 * j + i] ^ aux;
            }
        }
    }
    memcpy(state, tmp, 16);
}

void Cipher(unsigned char in[], unsigned char out[], unsigned char w[])
{
    unsigned char state[16];

    memcpy(state, in, 16);
    /*unsigned char h, l;
    cout << "StartOfRound\n";
    for (int i = 0; i < 16; i++)
    {
        h = state[i] / 16;
        l = state[i] % 16;
        if (h <= 9)
            h = h + '0';
        else
            h = h + 'a' - 10;
        if (l <= 9)
            l = l + '0';
        else
            l = l + 'a' - 10;
        cout << h;
        cout << l;
    }
    cout << '\n';*/
    AddRoundKey(state, w);
    /*cout << "RoundKey\n";
    for (int i = 0; i < 16; i++)
    {
        h = w[i] / 16;
        l = w[i] % 16;
        if (h <= 9)
            h = h + '0';
        else
            h = h + 'a' - 10;
        if (l <= 9)
            l = l + '0';
        else
            l = l + 'a' - 10;
        cout << h;
        cout << l;
    }
    cout << '\n';*/

    for (int round = 1; round <= Nr - 1; round++)
    {
        /*cout << "StartOfRound\n";
        for (int i = 0; i < 16; i++)
        {
            h = state[i] / 16;
            l = state[i] % 16;
            if (h <= 9)
                h = h + '0';
            else
                h = h + 'a' - 10;
            if (l <= 9)
                l = l + '0';
            else
                l = l + 'a' - 10;
            cout << h;
            cout << l;
        }
        cout << '\n';*/
        SubBytes(state);
        /*cout << "AfterSubBytes\n";
            for (int i = 0; i < 16; i++)
            {
                h = state[i] / 16;
                l = state[i] % 16;
                if (h <= 9)
                    h = h + '0';
                else
                    h = h + 'a' - 10;
                if (l <= 9)
                    l = l + '0';
                else
                    l = l + 'a' - 10;
                cout << h;
                cout << l;
            }
        cout << '\n';*/
        ShiftRows(state);
        /*cout << "AfterShiftRows\n";
            for (int i = 0; i < 16; i++)
            {
                h = state[i] / 16;
                l = state[i] % 16;
                if (h <= 9)
                    h = h + '0';
                else
                    h = h + 'a' - 10;
                if (l <= 9)
                    l = l + '0';
                else
                    l = l + 'a' - 10;
                cout << h;
                cout << l;
            }
        cout << '\n';*/
        MixColumns(state);
        /*cout << "AfterMixColumns\n";
            for (int i = 0; i < 16; i++)
            {
                h = state[i] / 16;
                l = state[i] % 16;
                if (h <= 9)
                    h = h + '0';
                else
                    h = h + 'a' - 10;
                if (l <= 9)
                    l = l + '0';
                else
                    l = l + 'a' - 10;
                cout << h;
                cout << l;
            }
        cout << '\n';*/
        AddRoundKey(state, w + round * Nb * 4);
        /*cout << "RoundKey\n";
            for (int i = 0; i < 16; i++)
            {
                h = w[round * Nb * 4 + i] / 16;
                l = w[round * Nb * 4 + i] % 16;
                if (h <= 9)
                    h = h + '0';
                else
                    h = h + 'a' - 10;
                if (l <= 9)
                    l = l + '0';
                else
                    l = l + 'a' - 10;
                cout << h;
                cout << l;
            }
        cout << '\n';*/
    }
    /*cout << "StartOfRound\n";
    for (int i = 0; i < 16; i++)
    {
        h = state[i] / 16;
        l = state[i] % 16;
        if (h <= 9)
            h = h + '0';
        else
            h = h + 'a' - 10;
        if (l <= 9)
            l = l + '0';
        else
            l = l + 'a' - 10;
        cout << h;
        cout << l;
    }
    cout << '\n';*/
    SubBytes(state);
    /*cout << "AfterSubBytes\n";
    for (int i = 0; i < 16; i++)
    {
        h = state[i] / 16;
        l = state[i] % 16;
        if (h <= 9)
            h = h + '0';
        else
            h = h + 'a' - 10;
        if (l <= 9)
            l = l + '0';
        else
            l = l + 'a' - 10;
        cout << h;
        cout << l;
    }
    cout << '\n';*/
    ShiftRows(state);
    /*cout << "AfterShiftRows\n";
    for (int i = 0; i < 16; i++)
    {
        h = state[i] / 16;
        l = state[i] % 16;
        if (h <= 9)
            h = h + '0';
        else
            h = h + 'a' - 10;
        if (l <= 9)
            l = l + '0';
        else
            l = l + 'a' - 10;
        cout << h;
        cout << l;
    }
    cout << '\n';*/
    AddRoundKey(state, w + Nr * Nb * 4);
    /*cout << "RoundKey\n";
        for (int i = 0; i < 16; i++)
        {
            h = w[Nr * Nb * 4 + i] / 16;
            l = w[Nr * Nb * 4 + i] % 16;
            if (h <= 9)
                h = h + '0';
            else
                h = h + 'a' - 10;
            if (l <= 9)
                l = l + '0';
            else
                l = l + 'a' - 10;
            cout << h;
            cout << l;
        }
    cout << '\n';*/
    memcpy(out, state, 16);
}

void InvShiftRows(unsigned char state[]) {
    unsigned char tmp[16];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < Nb; j++) {
            if (i - j >= 0)
                tmp[4 * i + j] = state[4 * (i - j) + j];
            else
                tmp[4 * i + j] = state[4 * (4 + i - j) + j];
        }
    }
    memcpy(state, tmp, 16);
}


void InvSubBytes(unsigned char state[]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < Nb; j++) {
            state[4 * i + j] = Isbox[state[4 * i + j] / 16][state[4 * i + j] % 16];
        }
    }
}

void InvMixColumns(unsigned char state[]) {
    unsigned char tmp[16];
    memset(tmp, 0, 16);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                int b = 1;
                unsigned char c = state[j * 4 + k], aux = 0;
                while (b <= IM[i][k])
                {
                    if ((b & IM[i][k]) != 0)
                    {
                        aux = aux ^ c;
                    }
                    c = xtime(c);
                    b = b << 1;
                }
                tmp[4 * j + i] = tmp[4 * j + i] ^ aux;
            }
        }
    }
    memcpy(state, tmp, 16);
}

void InvCipher(unsigned char in[], unsigned char out[], unsigned char w[])
{
    unsigned char state[16];

    memcpy(state, in, 16);
    /*unsigned char h, l;
    cout << "StartOfRound\n";
    for (int i = 0; i < 16; i++)
    {
        h = state[i] / 16;
        l = state[i] % 16;
        if (h <= 9)
            h = h + '0';
        else
            h = h + 'a' - 10;
        if (l <= 9)
            l = l + '0';
        else
            l = l + 'a' - 10;
        cout << h;
        cout << l;
    }
    cout << '\n';*/
    AddRoundKey(state, w + Nr * Nb * 4);
    /*cout << "RoundKey\n";
        for (int i = 0; i < 16; i++)
        {
            h = w[Nr * Nb * 4 + i] / 16;
            l = w[Nr * Nb * 4 + i] % 16;
            if (h <= 9)
                h = h + '0';
            else
                h = h + 'a' - 10;
            if (l <= 9)
                l = l + '0';
            else
                l = l + 'a' - 10;
            cout << h;
            cout << l;
        }
    cout << '\n';*/
    for (int round = Nr - 1; round >= 1; round--)
    {
        /*cout << "StartOfRound\n";
        for (int i = 0; i < 16; i++)
        {
            h = state[i] / 16;
            l = state[i] % 16;
            if (h <= 9)
                h = h + '0';
            else
                h = h + 'a' - 10;
            if (l <= 9)
                l = l + '0';
            else
                l = l + 'a' - 10;
            cout << h;
            cout << l;
        }
        cout << '\n';*/
        InvShiftRows(state);
        /*cout << "InvShiftRows\n";
        for (int i = 0; i < 16; i++)
        {
            h = state[i] / 16;
            l = state[i] % 16;
            if (h <= 9)
                h = h + '0';
            else
                h = h + 'a' - 10;
            if (l <= 9)
                l = l + '0';
            else
                l = l + 'a' - 10;
            cout << h;
            cout << l;
        }
        cout << '\n';*/
        InvSubBytes(state);
        /*cout << "InvSubBytes\n";
        for (int i = 0; i < 16; i++)
        {
            h = state[i] / 16;
            l = state[i] % 16;
            if (h <= 9)
                h = h + '0';
            else
                h = h + 'a' - 10;
            if (l <= 9)
                l = l + '0';
            else
                l = l + 'a' - 10;
            cout << h;
            cout << l;
        }
        cout << '\n';*/
        AddRoundKey(state, w + round * Nb * 4);
        /*cout << "RoundKey\n";
            for (int i = 0; i < 16; i++)
            {
                h = w[round * Nb * 4 + i] / 16;
                l = w[round * Nb * 4 + i] % 16;
                if (h <= 9)
                    h = h + '0';
                else
                    h = h + 'a' - 10;
                if (l <= 9)
                    l = l + '0';
                else
                    l = l + 'a' - 10;
                cout << h;
                cout << l;
            }
        cout << '\n';*/
        /*cout << "AfterRoundKey\n";
        for (int i = 0; i < 16; i++)
        {
            h = state[i] / 16;
            l = state[i] % 16;
            if (h <= 9)
                h = h + '0';
            else
                h = h + 'a' - 10;
            if (l <= 9)
                l = l + '0';
            else
                l = l + 'a' - 10;
            cout << h;
            cout << l;
        }
        cout << '\n';*/
        InvMixColumns(state);
        /*cout << "InvMixColumns\n";
        for (int i = 0; i < 16; i++)
        {
            h = state[i] / 16;
            l = state[i] % 16;
            if (h <= 9)
                h = h + '0';
            else
                h = h + 'a' - 10;
            if (l <= 9)
                l = l + '0';
            else
                l = l + 'a' - 10;
            cout << h;
            cout << l;
        }
        cout << '\n';*/
    }
    /*cout << "StartOfRound\n";
    for (int i = 0; i < 16; i++)
    {
        h = state[i] / 16;
        l = state[i] % 16;
        if (h <= 9)
            h = h + '0';
        else
            h = h + 'a' - 10;
        if (l <= 9)
            l = l + '0';
        else
            l = l + 'a' - 10;
        cout << h;
        cout << l;
    }
    cout << '\n';*/
    InvShiftRows(state);
    /*cout << "InvShiftRows\n";
    for (int i = 0; i < 16; i++)
    {
        h = state[i] / 16;
        l = state[i] % 16;
        if (h <= 9)
            h = h + '0';
        else
            h = h + 'a' - 10;
        if (l <= 9)
            l = l + '0';
        else
            l = l + 'a' - 10;
        cout << h;
        cout << l;
    }
    cout << '\n';*/
    InvSubBytes(state);
    /*cout << "InvSubBytes\n";
    for (int i = 0; i < 16; i++)
    {
        h = state[i] / 16;
        l = state[i] % 16;
        if (h <= 9)
            h = h + '0';
        else
            h = h + 'a' - 10;
        if (l <= 9)
            l = l + '0';
        else
            l = l + 'a' - 10;
        cout << h;
        cout << l;
    }
    cout << '\n';*/
    AddRoundKey(state, w);
    /*cout << "RoundKey\n";
        for (int i = 0; i < 16; i++)
        {
            h = w[i] / 16;
            l = w[i] % 16;
            if (h <= 9)
                h = h + '0';
            else
                h = h + 'a' - 10;
            if (l <= 9)
                l = l + '0';
            else
                l = l + 'a' - 10;
            cout << h;
            cout << l;
        }
    cout << '\n';*/
    memcpy(out, state, 16);
}

void encryptAES_BCD(unsigned char dataIN[], unsigned char dataOUT[], unsigned char key[]) {
    Nk = 8;
    Nb = 4;
    Nr = 14;
    unsigned char w[(14 + 1) * 4 * 4];
    KeyExpansion(key, w);
    Cipher(dataIN, dataOUT, w);
    //InvCipher(dataOUT, dataOUTT, w);
}

void decryptAES_BCD(unsigned char dataIN[], unsigned char dataOUT[], unsigned char key[]) {
    Nk = 8;
    Nb = 4;
    Nr = 14;
    unsigned char w[(14 + 1) * 4 * 4];
    KeyExpansion(key, w);
    InvCipher(dataIN, dataOUT, w);
}

/*int main0()
{
    int dim;
    Nk = 256 / 32;
    Nb = 4;
    unsigned char key[256] = "r5u8x/A?D(G+KbPeShVmYp3s6v9y$B&E";
    Nr = 14;
    unsigned char dataIN[4 * 4] = "" , dataOUT[4 * 4] = "";;
    int i = 0;
    char c = 'a';
    while (c != '\n')
    {
        cin.get(c);
        dataIN[i++] = c;
        if (i == 16)
            break;
    }
    unsigned char h, l;
    cout << "Mesaj decriptat: ";
    encryptAES(dataIN, dataOUT, key);
    decryptAES(dataOUT, dataIN, key);
    cout << dataIN;
    return 0;
}*/
