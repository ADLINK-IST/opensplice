import os, sys, pdb, fnmatch, re, shutil, cStringIO, optparse


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
            'TCM',
            #Streams
            'TStreamDataReader', 'TStreamDataWriter', 'TCorePolicy', 'TStreamSample', 'TStreamFlush'
            ]

    DSub =  [
            '<typename DELEGATE>', '<D>', '<DELEGATE>', 'template <typename DELEGATE>',
            'template <typename D>'
            ]
    TFileRegex = re.compile('^T(?!opic|ime).')

    #Used to find and replace *QoS.hpp
    #typedefs with regexed TEntity body
    QFileRegex = re.compile('(.*?)Qos\.hpp$')

    #The TEntityQos.hpp contents to regex
    TEntityQosFileContents = cStringIO.StringIO()

    def __init__(self, fpath, fname):
        self.fpath=fpath
        self.fname=fname
        self.original=open(fpath + os.path.sep + fname, 'r')
        self.newfile=cStringIO.StringIO()
        self.TEntityQos = False

        #Store the TEntity QoS in static memory for quick writing
        if 'TEntityQos.hpp' in self.fname:
            #Horrible hack because python doesnt like seek in a for line loop
            skip=0
            for line in self.original:
                #ignore predeclaration
                if re.match('namespace dds', line):
                    skip=8
                if skip > 0:
                    skip -=1
                else:
                    self.TEntityQosFileContents.write(line)

            self.TEntityQos = True

    def isT(self):
        if self.TFileRegex.match(self.fname):
            return True

    def isQos(self):
        if self.QFileRegex.match(self.fname) and not self.TEntityQos:
                return True

    def fixFilename(self):
        if self.isT():
            return self.fname[1:]

    def fixQoSFileContents(self):
        if self.isQos():
            #Get the class name
            classn = self.QFileRegex.match(self.fname)
            classn = classn.group(1)
            #Get the namespace
            NameSpace = ''
            for line in self.original:
                ns = re.match('typedef\s*(dds::.*?)::detail::', line)
                if not ns:
                    ns = re.match('typedef\s*(dds::streams::.*?)::detail::', line)
                if ns:
                    NameSpace = ns.group(1)
                    print "got NS " + NameSpace

            #Rewind from last regex (if any)
            self.TEntityQosFileContents.seek(0,0)
            for line in self.TEntityQosFileContents:
                line = line.replace('dds::core::TEntityQos', str(NameSpace) + '::' + classn + "Qos")
                line = line.replace('TEntityQos', classn + "Qos")
                self.newfile.write(line)

            #Rewind
            self.newfile.seek(0,0)
            return self.newfile


    def fixFileContents(self):

        for line in self.original:
            for sub in self.TSub:
                #Remove T from infront of class name
                line = line.replace(sub, sub[1:])
            for sub in self.DSub:
                #Remove DELEGATE and D template parameters
                line = re.sub(sub, '', line)
            self.newfile.write(line)

        #Rewind file
        self.newfile.seek(0,0)
        return self.newfile


    def copyFile(self, newpath):
        #Walk the directory tree backwards until "spec" is found
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
            #If its a QoS file eg. DomainParticpantQos.hpp then replace it with TEntityQos
            if self.isQos():
                print "Copying QoS file " + self.fname + " to " + newpath
                nf = open(os.path.join(newpath, self.fname), 'w')
                shutil.copyfileobj(self.fixQoSFileContents(), nf)

            else:
                print "Copying plain file " + self.fname + " to " + newpath
                shutil.copy(os.path.join(self.fpath, self.fname), os.path.join(newpath, self.fname))

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
    overwrite then with the contents of T files
    '''
    print "COPYING PLAIN FILES"
    for files in filelist:
        if not files.isT():
            files.copyFile(outputdir)

    print "COPYING T FILES"
    for files in filelist:
        if files.isT():
            files.copyFile(outputdir)

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


if __name__ == "__main__":
    main()
