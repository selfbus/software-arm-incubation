# knx2header config file

# xml file read
xmlIn = 'M-004C_A-0438-06-E129-O004C.xml'

# for script and output debugging
DBG = 0

# where to write to
outfile = '../src/params.h'
outTree = '../params.cpp'

# whether we want comments
comments = 1

# what language we want
outLang = 'en-US'
# outLang = 'de-DE'

# start of the file
headerStart = '''
#ifndef INC_LIGHTSENSOR_PARAMS_H
#define INC_LIGHTSENSOR_PARAMS_H

#define userEeprom (*(bcu.userEeprom))
'''

# end of the file
headerEnd = '''
#endif /* INC_LIGHTSENSOR_PARAMS_H */
'''

# indent of the tree body
treeIndent = '\t'

# start of the tree file
treeStart = ''

# end of the tree file
treeEnd = ''

# List of COM ids to ignore
# like [12,56,99,...]
comIgnoreLst = []

# List of ids to ignore everywhere except in the first position
ignoreLst = [
    1, 4, 10, 11, 30, 31,
    106, 107, 115, 122, 129, 141, 159, 165, 168, 171, 174, 178, 183, 187,
    213, 222, 227, 242, 251, 252, 273, 275, 291, 293, 296, 297,
    312, 304, 309, 337, 358, 360, 376, 378, 382, 389, 394, 336, 381, 397
]

# Numeric part of id that are to be completely ignored
ignoreFields = [305]

# list of sequences to suppress all but the first encountered
# Example: [ 1, 2, 3]
# When i see P-1 i ignore P-2 and P-3
# When i see P-2 i ignore only P-3
skiplists = [
    [267, 266],
    [210, 209],
    [352, 351],
    [393, 388, 380],
    [329, 323, 314],
    [308, 303, 295],
    [414, 408, 399],
    [146, 145, 144],
    [182, 177, 169],
    [152,  32, 149],
    [203, 197, 189]
]

# rename items k: P-123 v: text
refOveride = {
    # broken/missing
    'P-101': 'Timebase', 'P-102': 'Factor',
    'P-221': 'Factor', 'P-226': 'Timebase', 'P-286': 'Timebase',
    'P-289': 'Factor', 'P-371': 'Timebase', 'P-374': 'Factor',
    # more meaninful, eliminate duplicates
    'P-4': 'Type', 'P-5': 'T Sensor type',
    'P-133': 'Bus Recovery Threshold Temp Timefactor',
    'P-165': 'Type',
    'P-166': 'T Sensor type', 'P-251': 'Type', 'P-258': 'T Sensor type',
    'P-336': 'Type', 'P-343': 'T Sensor type',
}

