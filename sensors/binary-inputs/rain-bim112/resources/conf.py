# knx2header config file

# xml file read
xmlIn = 'M-0083_A-004F-11-8560.xml'

# for script and output debugging
DBG = 1

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
#ifndef INC_RAIN_PARAMS_H
#define INC_RAIN_PARAMS_H
'''

# end of the file
headerEnd = '''
#endif //INC_RAIN_PARAMS_H
'''

# indent of the tree body
treeIndent = '\t'

# start of the tree file
treeStart = ''

# end of the tree file
treeEnd = ''

# List of COM ids to ignore
# like [12,56,99,...]
comIgnoreLst = range(5, 16)

# List of ids to ignore everywhere except in the first position
ignoreLst = [
]

# Numeric part of id that are to be completely ignored
ignoreFields = []

# list of sequences to suppress all but the first encountered
# Example: [ 1, 2, 3]
# When i see P-1 i ignore P-2 and P-3
# When i see P-2 i ignore only P-3
skiplists = [
]

# rename items k: P-123 v: text
refOveride = {
    # broken/missing
    # more meaninful, eliminate duplicates
}

