# Your boss loves Bloom filters. To impress her, you start implementing one. 
# Your Bloom filter uses m=32 bits and 3 hash functions h1, h2, and h3, where hi(x) = ((x2 +x3)*i) mod m.
def main():
    hash_func = [lambda x, i=i: ((x**2 + x**3) * i) % 32 for i in range(1, 4)]
    m_table = [0] * 32
    for x in [2013, 2010, 2007]:
        for func in hash_func:
            print(func(x))
            m_table[func(x)] = 1
    print(m_table)

    #If someone checks for membership of the element 0
    # print(all(m_table[func(0)] == 1 for func in hash_func))
    print([func(3200) for func in hash_func])

if __name__ == '__main__':
    main()
