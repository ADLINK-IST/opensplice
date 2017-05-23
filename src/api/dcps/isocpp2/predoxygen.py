import os, sys, pdb, fnmatch, re, shutil, cStringIO, optparse


''' SafeEnum class '''
class SafeEnum:
    name_struct = ""
    name_typedef = ""

    def __init__(self, line):
        if 'typedef' in line and 'dds::core::safe_enum' in line:
            m = re.search('<(.+?)>', line)
            if m:
                self.name_struct = m.group(1)
            m = re.search('> *(.+?); *$', line)
            if m:
                self.name_typedef = m.group(1)

    def fixLine(self, line):
        if 'typedef' in line and 'dds::core::safe_enum' in line:
            # This typedef should be removed.
            line = ""
        else:
            # The name of the struct should be replaced by the typedef name
            line = line.replace(self.name_struct, self.name_typedef)
        return line

    def isValid(self):
        return ((len(self.name_struct) > 0) and (len(self.name_typedef) > 0))

    def toString(self):
        return "Struct({}), Typedef({})".format(self.name_struct, self.name_typedef)



''' FileObj class '''
class FileObj:
    #Static vars
    TSub =  [
            'TBuiltinTopic', 'TDomainParticipant', 'TEntity', 'TInstanceHandle', 'TQosProvider', 'TCondition',
            'TGuardCondtion', 'TStatusCondition', 'TWaitSet', 'TCorePolicy', 'TQosPolicy', 'TStatus',
            #XTypes
            'TAnnotation', 'TCollectionTypes', 'TDynamic', 'TMember', 'TStruct',
            'TType', 'TCollectionType', 'TExtensibilityAnnotation', 'TidAnnotation',
            'TKeyAnnotation', 'TPrimitiveType', 'TSequenceType', 'TStringType', 'TUnionForwardDeclaration',
            'TVerbatimAnnotation', 'TBitBoundAnnotation', 'TBitsetAnnotation', 'TMapType', 'TMustUnderstandAnnotation',
            'TNestedAnnotation', 'TIdAnnotation', 'TUnionForwardDeclaration',
            #End of XTypes
            'TDomainParticipant', 'TCoherentSet', 'TPublisher', 'TSuspendedPublication',
            'TCoherentAccess', 'TDataReader', 'TGenerationCount', 'TQuery', 'TRank',
            'TSample', 'TSubscriber', 'TReadCondition', 'TFilter', 'TGuardCondition', 'THolder', 'TDHolder',
            'TAnyDataReader', 'TAnyTopic', 'TAnyDataWriter',
            #QoS
            'TUserData', 'TGroupData', 'TTopic',
            'TTransportPriority', 'TLifespan', 'TDeadline', 'TLatencyBudget',
            'TTimeBasedFilter', 'TPartition', 'TOwnership', 'TWriterDataLifecycle',
            'TReaderDataLifecycle', 'TDurability', 'TPresentation','TReliability', 'TDestinationOrder',
            'THistory', 'TResourceLimits', 'TLiveliness', 'TDurabilityService', 'TShare', 'TProductData',
            'TSubscriptionKey', 'TDataRepresentation','TRequestedDeadlineMissedStatus','TInconsistentTopicStatus',
            'TOffered', 'TRequested',
            #TBuiltinStuff
            'TSubscription', 'TPublication', 'TParticipant', 'TTopicBuiltinTopicData',
            'TCM', 'TBuiltinTopicTypes'
            #Streams
            'TStreamDataReader', 'TStreamDataWriter', 'TCorePolicy', 'TStreamSample', 'TStreamFlush'
            ]

    DSub =  [
            'template <typename DELEGATE>', '<typename DELEGATE>', '<D>', '<DELEGATE>', '< DELEGATE >',
            'template <typename D>', '< DELEGATE<T> >', ', template <typename Q> class DELEGATE', ', DELEGATE'
            ]

    Tfiles = [
             'DataWriter.hpp', 'DataWriterImpl.hpp',
             'LoanedSamples.hpp', 'LoanedSamplesImpl.hpp',
             'SharedSamples.hpp', 'SharedSamplesImpl.hpp',
             'UnionCase.hpp', 'Optional.hpp'
             ]

    TFileRegex = re.compile('^T(?!opic|ime|ype).')

    #Used to find and replace *QoS.hpp
    QFileRegex = re.compile('(.*?)Qos\.hpp$')

    def __init__(self, fpath, fname):
        self.fpath=fpath
        self.fname=fname
        self.original=open(fpath + os.path.sep + fname, 'r')
        self.newfile=cStringIO.StringIO()
        self.getSafeEnums()

    def getSafeEnums(self):
        self.safeEnums = []
        for line in self.original:
            safeEnum = SafeEnum(line)
            if safeEnum.isValid():
                self.safeEnums.append(safeEnum)
        self.original.seek(0,0)

    def isTbyName(self):
        # Most template files start their name with a 'T'
        if self.TFileRegex.match(self.fname):
            return True
        return False

    def isTbyContent(self):
        # Some template files don't start their name with a 'T'
        for filename in self.Tfiles:
            if self.fname == filename:
                return True
        return False

    def isT(self):
        if self.isTbyName():
            return True
        if self.isTbyContent():
            return True
        return False

    def isQos(self):
        if self.QFileRegex.match(self.fname):
            # EntityQos container is not a real QoS and should not be treated as such.
            if 'TEntityQos.hpp' not in self.fname:
                return True
        return False

    def isDataWriter(self):
        if self.fname == "DataWriter.hpp" or self.fname == "DataWriterImpl.hpp":
            return True
        return False

    def fixFilename(self):
        if self.isTbyName():
            return self.fname[1:]
        return self.fname


    def fixFileContents(self):
        for line in self.original:
            for sub in self.TSub:
                #Remove T from infront of class name
                line = line.replace(sub, sub[1:])
            for sub in self.DSub:
                #Remove DELEGATE and D template parameters
                line = re.sub(sub, '', line)
            for safeEnum in self.safeEnums:
                #Fix safe enums
                line = safeEnum.fixLine(line)
            self.newfile.write(line)

        #Rewind file
        self.newfile.seek(0,0)
        return self.newfile


    def copyFile(self, newpath):
        #Walk the directory tree backwards until "include" is found
        relpath = ''
        path = [self.fpath, '']
        while True:
            path = os.path.split(path[0])
            relpath = os.path.join(path[1], relpath)
            if "include" in relpath:
                break
        newpath = os.path.join(newpath, relpath)
        #Create the directory if it doesnt exist
        if not os.path.exists(newpath):
            os.makedirs(newpath)
            print "Creating Dir: " + newpath
        #Copy the file and fix the filename if its a Tfile
        if not self.isT():
            #If its a QoS file eg. DomainParticpantQos.hpp then replace it with its detail counterpart
            if self.isQos():
                if "detail" not in self.fpath and "detail" not in newpath:
                    print "Copying QoS file " + self.fname + " (from detail directory) to " + newpath
                    shutil.copy(os.path.join(self.fpath, "detail/", self.fname), os.path.join(newpath, self.fname))
                else:
                    print "Ignore " + self.fname

            else:
                if len(self.safeEnums) == 0:
                    print "Copying plain file " + self.fname + " to " + newpath
                    shutil.copy(os.path.join(self.fpath, self.fname), os.path.join(newpath, self.fname))
                else:
                    print "{}".format(self.safeEnums)
                    print "Copying plain file (with safe_enums) " + self.fname + " to " + newpath
                    nf = open(os.path.join(newpath, self.fname), 'w')
                    shutil.copyfileobj(self.fixFileContents(), nf)
        else:
            nf = open(os.path.join(newpath, self.fixFilename()), 'w')
            shutil.copyfileobj(self.fixFileContents(), nf)
            print "Copying T file " + self.fname + " to " + newpath + self.fixFilename()


def getFileList(basedirs, filetype):
    matches = []
    for basedir in basedirs:
        print "Looking in " + basedir + " for " + filetype
        for root, dirnames, filenames in os.walk(basedir):
            for filename in fnmatch.filter(filenames, filetype):
                matches.append(FileObj(root, filename))
        print "Found " + str(len(matches)) + " files"
    return matches


def copyFiles(filelist, outputdir):
    '''
    copy all the none T files first, then
    overwrite them with the contents of T files
    '''
    print "COPYING PLAIN FILES"
    for files in filelist:
        if not files.isT():
            files.copyFile(outputdir)

    print "COPYING T FILES"
    for files in filelist:
        if files.isT():
            files.copyFile(outputdir)

def deleteEmptyDirs(outputdir):
    for dirpath, _, _ in os.walk(outputdir, topdown=False):
        if dirpath == outputdir:
            break
        if not os.listdir(dirpath):
            print "Remove empty dir: {}".format(dirpath)
            os.rmdir(dirpath)

def main():
    parser = optparse.OptionParser()

    parser.add_option("-i", "--inputdir", dest="basedir", action="append",
            help=("Base folder for the documents eg. $OSPL_HOME/src/api/dcps/isocpp/include")
            )
    parser.add_option("-e", "--extension", dest="fx",
            help=("File extension eg. hpp"
                "default is hpp"),
            default="hpp")

    parser.add_option("-o", "--outputdir", dest="outputdir",
            help=("Output directory")
            )

    (options, args) = parser.parse_args()

    copyFiles(getFileList(options.basedir,'*.' + options.fx), options.outputdir)

    deleteEmptyDirs(options.outputdir)


if __name__ == "__main__":
    main()
