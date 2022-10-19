#!/bin/python

import sys,os,subprocess,argparse,json,threading,time,copy

RED = 1
GREEN = 2
YELLOW = 3
BLUE = 4
MAGENTA = 5
CYAN = 6
WHITE = 7

noColor = False

def GetPlatform():
	if sys.platform == 'win32' or sys.platform == 'cygwin':
		return 'windows'
	if sys.platform == 'darwin':
		return 'macOS'
	return sys.platform

def RESET():
    global noColor
    if noColor:
        return ''

    return '\x1b[0m'

def TextColor(color,bold=0):
    global noColor
    if noColor:
        return ''

    return f'\x1b[{bold};{30+color}m'
    
def ERROR():
    return TextColor(RED,1)
    
def MODE():
    return TextColor(CYAN,1)
    
def GetNumSize(num):
    c = 1
    while num>9:
        num //= 10
        c+=1
    return c

def IsObjFileOutdated(srcFile,objFile):
    return GetFileTime(srcFile)>=GetFileTime(objFile)

def GetFileTime(filename):
    if not os.path.exists(filename):
        return 0
    
    return int(os.path.getmtime(filename))

def MakePath(path): #path guaranteed does not exist
	print(f'{TextColor(YELLOW)}Creating {path}{RESET()}')
	MakePathSub(path)
	
def MakePathSub(path):
    path = os.path.normpath(path)
    upper = os.path.dirname(path)
    if not os.path.exists(upper) and upper:
        MakePathSub(upper)
    os.mkdir(path)

def SortByFileTimesIP(files):
    files.sort(reverse=True,key=GetFileTime)

def GetFileSizes(files):
    s = 0
    for file in files:
        s += os.path.getsize(file)

    return s

def GetExtension(filename):
    return os.path.splitext(filename)[1][1:]

def SetExtension(filename,ext):
    return os.path.splitext(filename)[0]+'.'+ext

def AddExtension(filename,ext):
    return filename+'.'+ext

def GetPrefixAndName(path):
	return os.path.dirname(path),os.path.basename(path)

def CPPExtractIncludeFile(line):
    start = line.find('"')
    end = line.find('"',start+1)
    return line[start+1:end]

def CPPDeps(path,includeDir=''):
    deps = set()
    prefix,filename = GetPrefixAndName(path)

    with open(path,'r') as f:
        for line in f:
            line = line.lstrip(' \t')
            if line[0]!='#':
                continue
            line = line[1:].lstrip(' \t')
            if line.startswith('include') and '"' in line:
                dep = CPPExtractIncludeFile(line)
                depprefix = os.path.join(prefix,dep)
                if not os.path.exists(depprefix) and includeDir!='':
                    depprefix = os.path.join(includeDir,dep)
                deps.add(os.path.normpath(depprefix))

    return deps

class Builder:
    def __init__(self,options):
        self.options = options
        self.TestDirs([],self.options['modes'])
        
        self.depExtractFunc = None
        self.depdict = {}
        self.invdict = {}
        self.compileFiles = set()
        self.rebuildList = []
        self.debug = False
        self.quiet = False
        self.single = False

        self.commandFailed = False
        self.failLock = threading.Lock()
        self.printLock = threading.Lock()
        self.dispatchLock = threading.Lock()

    def DebugPrint(self,msg,end='\n'):
        if self.debug:
            self.InfoPrint(msg,end)

    def InfoPrint(self,msg,end='\n'):
        if not self.quiet:
            print(msg,flush=True,end=end)

    def ThreadedPrint(self,msg,end='\n'):
        with self.printLock:
            self.InfoPrint(msg,end)
    
    def GetSourceExts(self,mode):
        exts = GetModeVar(self.options,mode,'sourceExt')
        if type(exts)==str:
            exts = [exts]
        
        return exts
    
    def GetHeaderExts(self,mode):
        exts = GetModeVar(self.options,mode,'headerExt')
        if type(exts)==str:
            exts = [exts]
        
        return exts

    def RemoveObjects(self,path,ext):
        l = os.listdir(path)
        for f in l:
            p = os.path.join(path,f)
            if os.path.isdir(p):
                self.RemoveObjects(p,ext)
            elif os.path.isfile(p):
                if GetExtension(f)==ext:
                    os.remove(p)
                    self.DebugPrint(f"{TextColor(MAGENTA)}Deleting {p}...{RESET()}")
		
    def DirContainsObjectsSub(self,path,ext):
        l = os.listdir(path)
        for f in l:
            p = os.path.join(path,f)
            if os.path.isdir(p):
                if self.DirContainsObjectsSub(p,ext):
                    return True
            elif os.path.isfile(p):
                if GetExtension(f)==ext:
                    return True

        return False
    
    def DirContainsObjects(self,mode):
        path = self.GetPath(mode,'objectDir')
        ext = GetModeVar(self.options,mode,'objectExt')
        
        if not os.path.exists(path):
            return False
        
        return self.DirContainsObjectsSub(path,ext)

    def FindFileDependencies(self,path,includeDir=''):
        if not os.path.exists(path):
            return
		
        deps = self.depExtractFunc(path,includeDir)
        self.depdict[path] = deps
        for d in deps:
            if d not in self.depdict: #if dependency not tracked, add it and recursively search for more deps
                self.FindFileDependencies(d,includeDir)
    
    def InvertDependencies(self):
        self.invdict = {}
        for file in self.depdict:
            for dep in self.depdict[file]:
                if dep not in self.invdict:
                    self.invdict[dep] = set()
                self.invdict[dep].add(file)
        return self.invdict

    def HeaderFileCascade(self,mode,headerFile,cascadeSet=None,checkedHeadersSet=None): #return all source files affected by a header file
        if cascadeSet == None:
            cascadeSet = set()
            checkedHeadersSet = set()

        children = self.invdict[headerFile]
        sourceExts = self.GetSourceExts(mode)
        headerExts = self.GetHeaderExts(mode)

        for child in children:
            if GetExtension(child) in sourceExts:
                cascadeSet.add(child)
            elif GetExtension(child) in headerExts:
                if child not in checkedHeadersSet:
                    checkedHeadersSet.add(child)
                    cascadeSet = self.HeaderFileCascade(mode,child,cascadeSet,checkedHeadersSet)

        return cascadeSet
        
    def CollectCompilables(self,srcDir,srcExts,includeDir=''):
        files = os.listdir(srcDir)
        for file in files:
            path = os.path.join(srcDir,file)
            if os.path.isdir(path):
                self.CollectCompilables(path,srcExts,includeDir)
            elif GetExtension(file) in srcExts:
                self.compileFiles.add(path)
                self.FindFileDependencies(path,includeDir)
    
    def CollectAllCompilables(self,mode,srcDir,srcExts):
        self.compileFiles = set()
        self.depdict = {}
        self.invdict = {}
        self.rebuildList = []
        includeDir = self.GetPath(mode,'includeDir')
        self.CollectCompilables(srcDir,srcExts,includeDir)
        self.DebugPrint(f"Found {len(self.compileFiles)} source files.")
        self.DebugPrint(f"Tracked {len(self.depdict)} total dependencies.")

    def GetRebuildSet(self,mode):
        self.rebuildSet = set()

        for srcFile in self.compileFiles:
            objFile = self.GetObjectFromSource(mode,srcFile)
            if IsObjFileOutdated(srcFile,objFile):
                self.rebuildSet.add(srcFile)
                self.DebugPrint(f"Adding source file {srcFile}\nReason: outdated object")

        outputPath = self.GetOutputPath(mode)
        outputAge = GetFileTime(outputPath)
        self.DebugPrint(f'Output ({outputPath}) has age {outputAge}')
        
        includeDir = GetModeVar(self.options,mode,'includeDir')
        if includeDir!=None:
            includeDir = self.GetPath(mode,'includeDir')

        for headerFile in self.invdict:
            headerAge = GetFileTime(headerFile)
            if headerAge==0 and includeDir!=None:
                headerAge = GetFileTime(os.path.join(includeDir,headerFile))
            
            if headerAge>=outputAge:
                self.DebugPrint(f'Cascading {headerFile}...')
                headerSet = self.HeaderFileCascade(mode,headerFile)
                for srcFile in headerSet:
                    objFile = self.GetObjectFromSource(mode,srcFile)
                    if headerAge>=GetFileTime(objFile):
                        if self.debug:
                            if srcFile not in self.rebuildSet:
                                self.DebugPrint(f"Adding source file {srcFile}\nReason: found in outdated header cascade")
                        self.rebuildSet.add(srcFile)

        self.rebuildList = list(self.rebuildSet)
        SortByFileTimesIP(self.rebuildList)

    def GetObjectPaths(self,mode):
        d = self.GetPath(mode,'objectDir')
        s = ''
        for file in self.compileFiles:
            p = self.GetObjectFromSource(mode,file)
            s += p+' '
            
        return s[:-1]

    def GetObjectFromSource(self,mode,src):
        d = self.GetPath(mode,'objectDir')
        return os.path.join(d,AddExtension(src,GetModeVar(self.options,mode,'objectExt')))

    def GetOutputPath(self,mode):
        d = self.GetPath(mode,'outputDir')
        return os.path.join(d,GetModeVar(self.options,mode,'outputName'))

    def GetBuilderPath(self):
        return os.path.abspath(__file__)
        
    def TestDirs(self,mode,d):
        for name,submode in d.items():
            real = mode+[name]
            if submode['modes']:
                self.TestDirs(real,submode['modes'])
                continue
                
            if GetModeVar(self.options,real,'linkCmd'):
                test = self.GetPath(real,'outputDir')
                if not os.path.exists(test):
                    MakePath(test)
            
            if GetModeVar(self.options,real,'compileCmd'):
                test = self.GetPath(real,'objectDir')
                if not os.path.exists(test):
                    MakePath(test)
            
            test = self.GetPath(real,'sourceDir')
            if not os.path.exists(test):
                MakePath(test)
                
    def GetPath(self,mode,name):
        var = GetModeVar(self.options,mode,name)
        properMode = GetModeMode(self.options,mode,name)
        if type(var) is list:
            var = self.FlagListPreprocess(properMode,name,var)
        
        return self.ResolvePath(mode,var)
    
    def GetCommand(self,mode,name,infile='%in',outfile='%out'):
        var = GetModeVar(self.options,mode,name)
        properMode = GetModeMode(self.options,mode,name)
        if type(var) is list:
            var = self.FlagListPreprocess(properMode,name,var)
        
        return self.GetCommandFlags(mode,var,infile,outfile)
        
    def FlagListPreprocess(self,mode,name,flagList):
        newList = []
        for flag in flagList:
            if flag[1:]==name:
                v = GetModeVar(self.options,mode[:-1],name)
                higher = GetModeMode(self.options,mode[:-1],name)
                if v is None:
                    print(f"{ERROR()}Flag '{name}' in mode {MODE()}{ModeStr(mode)}{ERROR()} is not present in a higher scope!")
                    ErrorExit()
                
                ext = self.FlagListPreprocess(higher,name,v)
                newList.extend(ext)
            else:
                newList.append(flag)
        
        return newList

    def ResolveFlag(self,mode,flag,inFlag='%in',outFlag='%out'):
        if flag[:2]=='\\%': return flag[1:] # escaped flag name
        if flag=='%mode':
            return ModeStr(mode)
        if flag=='%modePath':
            return ModeStr(mode).replace('/',os.path.sep)
        if flag=='%modeLast':
            return mode[-1]
        if flag=='%modeFirst':
            return mode[0]
        if flag=='%self':
            return self.GetBuilderPath()
        if flag=='%utime':
            return str(int(time.time()))
        if flag=='%platform':
            return GetPlatform()
        
        var = GetModeVar(self.options,mode,flag[1:])
        if var!=None:
            if type(var) is list:
                proper = GetModeMode(self.options,mode,flag[1:])
                nv = self.FlagListPreprocess(proper,flag[1:],var)
                if 'Dir' in flag:
                    return self.ResolvePath(mode,nv)
                return self.GetCommandFlags(mode,nv,inFlag,outFlag)
                
            return str(var)

        print(f"{ERROR()}Unexpected special flag '{flag}'!")
        ErrorExit()
       
    def ResolvePath(self,mode,pathList):
        if type(pathList) is str:
            return pathList # pathList is already a string
    
        s = ''
        concat = False
        for d in pathList:
            concat = False
            
            if d=='':
                continue
            
            if d[0]=='#' or d[:2]=='\\#':
                concat = d[0]=='#'
                d = d[1:]
                
            if d[0]=='%' or d[:2]=='\\%':
                d = self.ResolveFlag(mode,d)
            
            if concat and s!=os.path.sep:
                s = s[:-1] # remove trailing /
            
            if d=='':
                continue
                
            s += d + os.path.sep
        
        if not s:
            s = '.'
	
        return s
    
    def GetCommandFlags(self,mode,flags,infile,outfile):
        if type(flags) is str:
            return flags
            
        cmd = ''
        concat = False
        for flag in flags:
            concat = False
            if flag == '':
                continue

            if flag[0] == '#' or flag[:2] == '\\#':
                concat = flag[0]=='#'
                flag = flag[1:]
				
            if flag[0] == '%' or flag[:2] == '\\%':
                if flag[1:] == 'out':
                    flag = outfile
                elif flag[1:] == 'in':
                    flag = infile
                else:
                    flag = self.ResolveFlag(mode,flag,infile,outfile)
            
            if flag == '':
                continue
            
            if not concat:
                cmd += ' '
            
            cmd += flag
            
        return cmd.lstrip()

    def GetCompileCommand(self,mode,file):
        objVersion = self.GetObjectFromSource(mode,file)
        return self.GetCommand(mode,'compileCmd',file,objVersion)

    def GetLinkCommand(self,mode):
        inputFiles = self.GetObjectPaths(mode)
        outputFile = self.GetOutputPath(mode)
        return self.GetCommand(mode,'linkCmd',inputFiles,outputFile)
    
    def RunCommand(self,cmd):
        p = subprocess.Popen(cmd,stdout=sys.stdout,stderr=sys.stderr,shell=True)
        return p.wait()

    def Scan(self,mode):
        if GetModeVar(self.options,mode,'compileCmd') or GetModeVar(self.options,mode,'linkCmd'):
            self.GetDepExtractFunc(mode)
            srcDir = self.GetPath(mode,'sourceDir')
            self.CollectAllCompilables(mode,srcDir,self.GetSourceExts(mode))
            self.InvertDependencies()
            self.GetRebuildSet(mode)
            
    def RequestCommand(self):
        with self.dispatchLock:
            if self.dispatchedCommands:
                return True,self.dispatchedCommands.pop(0)
        
        return False,None
	
    def BuildObjectsFromList(self,totalCount):
        code = 0
        threadName = threading.current_thread().name
        while True:
            status,req = self.RequestCommand()
            if not status:
                break
            if self.HasCommandFailed():
                quit()
                
            src,obj,cmd,index = req[0],req[1],req[2],req[3]
            objDir = os.path.dirname(obj)
            if not os.path.exists(objDir):
                if self.debug:
                    self.ThreadedPrint(f"{TextColor(MAGENTA)}Created build path {objDir}{RESET()}")
                MakePathSub(objDir)
                
            if self.debug:
                self.ThreadedPrint(f"{TextColor(BLUE)}{cmd}{RESET()}")
            else:
                self.ThreadedPrint(f'{TextColor(WHITE,1)}[{MODE()}{threadName}{TextColor(WHITE,1)}] {TextColor(GREEN)}Building ({index+1}/{totalCount}): {TextColor(YELLOW)}{src} {TextColor(WHITE,1)}-> {TextColor(BLUE)}{obj}{RESET()}')

            code = self.RunCommand(cmd)
            if code!=0:
                self.SetCommandFailed()
                break
                
        return code

    def HasCommandFailed(self):
        with self.failLock:
            return self.commandFailed

    def SetCommandFailed(self):
        with self.failLock:
            self.commandFailed = True

    def CommandFailedQuit(self):
        if self.HasCommandFailed():
            self.ThreadedPrint(f"{ERROR()}Not all files were successfully compiled!")
            ErrorExit()
            quit()

    def DispatchCommands(self,cmdList,totalCount):
        cores = os.cpu_count()
        if self.single:
            cores = 1
        self.dispatchedCommands = cmdList

        for i in range(cores):
            thread = threading.Thread(target=self.BuildObjectsFromList,args=(totalCount,),name=str(i+1))
            thread.start()
            if self.single:
                thread.join()
        
        while threading.active_count()!=1:
            if self.HasCommandFailed():
                quit()
            time.sleep(0.25)

        if self.HasCommandFailed():
            self.CommandFailedQuit()

    def GetDefaultMode(self,op):
        m = op['defaultMode']
        if m=='%platform':
            m = GetPlatform()
        return m

    def FixMode(self,mode):
        if mode==[]:
            mode = [self.GetDefaultMode(self.options)]
        curr = self.options
        for submode in mode:
            if submode not in curr['modes']:
                self.ModeNotFoundError(mode)
            curr = curr['modes'][submode]
        
        while 'defaultMode' in curr:
            df = self.GetDefaultMode(curr)
            mode.append(df)
            curr = curr['modes'][df]
        
        return mode

    def ModeNotFoundError(self,mode):
        self.InfoPrint(f"{ERROR()}Mode {MODE()}{ModeStr(mode)}{ERROR()} not found!")
        ErrorExit()

    def GetCommands(self,mode,cmdList):
        properCmds = []
        for cmd in cmdList:
            if type(cmd) is list:
                builtCmd = self.GetCommandFlags(mode,cmd,'%in','%out')
                properCmds.append(builtCmd)
            elif type(cmd) is str:
                if cmd[0]=='%':
                    cmd = self.ResolveFlag(mode,cmd)
                    properCmds.append(cmd)
                else:
                    properCmds.append(cmd)
                
        return properCmds

    def GetPreCommands(self,mode):
        cmds = GetModeVar(self.options,mode,'preCmds')
        return self.GetCommands(mode,cmds)

    def GetPostCommands(self,mode):
        cmds = GetModeVar(self.options,mode,'postCmds')
        return self.GetCommands(mode,cmds)

    def Done(self):
        self.InfoPrint(f'{TextColor(WHITE,1)}Done!{RESET()}')

    def GetDepExtractFunc(self,mode):
        self.depExtractFunc = CPPDeps
        
    def PruneObjectsSub(self,path,ext):
        files = os.listdir(path)
        for file in files:
            p = os.path.join(path,file)
            if os.path.isdir(p):
                self.PruneObjectsSub(p,ext)
            elif os.path.isfile(p):
                if GetExtension(file)==ext:
                    prune = False
                    if os.path.getsize(p)==0:
                        prune = True
                        self.DebugPrint(f"Pruned zero-size object: {p}")
                    else:
                        src = os.path.splitext(p)[0]
                        found = False
                        for srcFile in self.compileFiles:
                            if src.endswith(srcFile):
                                found = True
                                break
                        if not found:
                            prune = True
                            self.DebugPrint(f"Pruned object with no corresponding source: {p}")
            
                    if prune:
                        self.DebugPrint(f"Removing {p}")
                        os.remove(p)
                        self.pruned = True
        
    def PruneObjects(self,mode):
        self.pruned = False
        objDir = self.GetPath(mode,'objectDir')
        objExt = GetModeVar(self.options,mode,'objectExt')
        
        self.PruneObjectsSub(objDir,objExt)
        
        if self.pruned:
            self.Scan(mode)

    def IsBlankMode(self,mode):
        cc = GetModeVar(self.options,mode,'compileCmd')
        if cc:
            return False

        lc = GetModeVar(self.options,mode,'linkCmd')
        if lc:
            return False

        prec = GetModeVar(self.options,mode,'preCmds')
        if prec:
            return False

        postc = GetModeVar(self.options,mode,'postCmds')
        if postc:
            return False

        return True

    def Build(self,mode):
        if mode==[]:
            mode = self.FixMode(mode)
            self.InfoPrint(f"{TextColor(WHITE,1)}Using default mode {MODE()}{ModeStr(mode)}{RESET()}")
        else:
            mode = self.FixMode(mode)
            if not self.IsBlankMode(mode):
                self.InfoPrint(f"{TextColor(WHITE,1)}Using mode {MODE()}{ModeStr(mode)}{RESET()}")

        preCmds = self.GetPreCommands(mode)
        postCmds = self.GetPostCommands(mode)
        code = 0
        
        settings = GetModeVar(self.options,mode,'set')
        if settings:
            for key,value in settings.items():
                if key[0]=='%' or key[:2]=='\\%':
                    key = self.ResolveFlag(mode,key)
                self.options[key] = value

        for command in preCmds:
            self.DebugPrint(f"{TextColor(MAGENTA)}{command}{RESET()}")
            code = self.RunCommand(command)
            if code!=0:
                ErrorExit()

        self.Scan(mode)

        errored = False
        cmd = ''
        
        if self.DirContainsObjects(mode):
            self.PruneObjects(mode)
            
        compileCount = len(self.rebuildList)
        # compilation
        if compileCount!=0 and GetModeVar(self.options,mode,'compileCmd')!='':
            self.InfoPrint(f'{TextColor(WHITE,1)}Building {MODE()}{compileCount}{TextColor(WHITE,1)} files...')
            
            cmdList = [(file,self.GetObjectFromSource(mode,file),self.GetCompileCommand(mode,file),i) for i,file in enumerate(self.rebuildList)]
            
            self.DispatchCommands(cmdList,compileCount)
            if errored:
                self.InfoPrint(f"{ERROR()}Not all files were successfully compiled!")
                ErrorExit()
                
        # linking
        if GetModeVar(self.options,mode,'linkCmd')!='':
            self.InfoPrint(f'{TextColor(WHITE,1)}Linking executable...{RESET()}')

            cmd = self.GetLinkCommand(mode)
            if self.debug:
                self.InfoPrint(f'{TextColor(BLUE)}{cmd}{RESET()}')
            else:
                src = os.path.normpath(self.GetPath(mode,'objectDir'))
                dest = self.GetOutputPath(mode)
                self.InfoPrint(f'{TextColor(GREEN)}Linking: {TextColor(BLUE)}{src} {TextColor(WHITE,1)}-> {TextColor(GREEN,1)}{dest}{RESET()}')
            code = self.RunCommand(cmd)

            if code!=0:
                self.InfoPrint(f"{ERROR()}Linker error!")
                ErrorExit()

        for command in postCmds:
            self.InfoPrint(f"{TextColor(MAGENTA)}{command}{RESET()}")
            code = self.RunCommand(command)
            if code!=0:
                ErrorExit()

        if not self.IsBlankMode(mode):
            self.Done()
            
    def ModeExists(self,mode):
        curr = self.options['modes']
        for submode in mode:
            if submode not in curr:
                return False
            curr = curr[submode]['modes']
        
        return True
        
    def NeedsCleaning(self,mode):
        return self.DirContainsObjects(mode) or os.path.exists(self.GetOutputPath(mode))
    
    def Clean(self,m):
        subs = GetAllSubModes(GetModeDict(self.options,m)['modes'],m)
        if not subs:
            subs = [m]
            
        for mode in subs:
            ext = GetModeVar(self.options,mode,'objectExt')
            if self.NeedsCleaning(mode):
                self.InfoPrint(f"{TextColor(WHITE,1)}Using mode {MODE()}{ModeStr(mode)}{RESET()}")
                self.InfoPrint(f"{TextColor(WHITE,1)}Cleaning up...{TextColor(YELLOW)}")
                if self.DirContainsObjects(mode):
                    path = self.GetPath(mode,'objectDir')
                    self.InfoPrint(f"{TextColor(YELLOW)}Removing objects in {path}")
                    self.RemoveObjects(path,ext)
                
                path = self.GetOutputPath(mode)
                if os.path.exists(path):
                    self.InfoPrint(f"{TextColor(YELLOW)}Removing {path}")
                    os.remove(path)
		
                self.Done()

    def Stats(self,mode):
        mode = self.FixMode(mode)
        
        self.InfoPrint(f"{TextColor(WHITE,1)}Using mode {MODE()}{ModeStr(mode)}{RESET()}")

        self.Scan(mode)
        fileCount = len(self.depdict)
        sourceCount = len(self.compileFiles)
        totalSize = GetFileSizes(self.depdict.keys())//1024

        justSize = max(GetNumSize(totalSize),GetNumSize(fileCount),GetNumSize(sourceCount))+1

        self.InfoPrint(f"{TextColor(WHITE,1)}Project Stats:")
        self.InfoPrint(f"{TextColor(YELLOW)}File count:   {MODE()}{str(fileCount).rjust(justSize)}")
        self.InfoPrint(f"{TextColor(YELLOW)}Source count: {MODE()}{str(sourceCount).rjust(justSize)}\n")
        
        self.InfoPrint(f"{TextColor(YELLOW,1)}Code size:    {TextColor(GREEN,1)}{str(totalSize).rjust(justSize)}{TextColor(WHITE,1)}K{RESET()}")
    
    def List(self,mode):
        self.FixMode(mode.copy())
        allModes = GetAllSubModes(GetModeDict(self.options,mode)['modes'],mode)
        if allModes == []:
            allModes = [mode]
            
        self.InfoPrint(f"{MODE()}",end='')
        for m in allModes:
            self.InfoPrint(ModeStr(m))
        self.InfoPrint(RESET(),end='')


def ExitingMsg():
    return f"{ERROR()}Exiting...{RESET()}"

def ErrorExit():
    print(ExitingMsg())
    quit(1)

def GetModeDict(options,mode): # return the dict corresponding to this mode/submode
    curr = options['modes']
    d = options
    for submode in mode:
        d = curr[submode]
        curr = curr[submode]['modes']
    return d
    
def GetModeMode(options,mode,varName): # return the most specific mode containing variable varName
	best = None
	if varName in options:
		best = []
	
	curr = options['modes']
	working = []
	for submode in mode:
		working.append(submode)
		if varName in curr[submode]:
			best = working.copy()
		curr = curr[submode]['modes']
	return best

def GetModeVar(options,mode,varName): # return a mode var, falling back to the root dict if not available in mode
    best = None
    if varName in options:
        best = options[varName]

    curr = options['modes']
    for submode in mode:
        if varName in curr[submode]:
            best = curr[submode][varName]
        curr = curr[submode]['modes']
    return best

def FixDirs(options):
    dirs = ['sourceDir','objectDir','outputDir']

    for d in dirs:
        modes = options['modes']
        for mode in modes:
            if d in modes[mode]:
                if modes[mode][d]=='':
                    modes[mode][d] = '.'
        if d in options:
            if options[d]=='':
                options[d] = '.'
                
def GetAllSubModes(modeDict,mode):
    l = []
    for name,submode in modeDict.items():
        if submode['modes']:
            l.extend(GetAllSubModes(submode['modes'],mode+[name]))
        else:
            l.append(mode+[name])
    return l

def SetDefaults(op,defaults):
    for opName,default in defaults:
        if opName not in op:
            op[opName] = default
            
def ModeStr(mode):
	return '/'.join(mode)
    
def ParseArgModes(modes):
    new = []
    for mode in modes:
        if mode=='':
            new.append([])
        split = mode.split('/')
        if '' in split:
            print(f"{ERROR()}Malformed mode {MODE()}{mode}{ERROR()} found!")
            ErrorExit()
        new.append(split)
    return new
    
def CreateSubModeDicts(modes,history=[]):
    for name,mode in modes.items():
        if 'modes' not in mode:
            mode['modes'] = {}
        elif 'modes' in mode and not mode['modes']:
            print(f"{ERROR()}Empty 'modes' dict found in {MODE()}{ModeStr(history+[name])}{ERROR()}!")
            ErrorExit()
        else:
            if 'defaultMode' not in mode:
                mode['defaultMode'] = list(mode['modes'].keys())[0]
            elif '/' in mode['defaultMode']:
                print(f"{ERROR()}'defaultMode' var cannot contain '/' (in {MODE()}{ModeStr(history+[name])}{ERROR()}!")
                ErrorExit()
            CreateSubModeDicts(mode['modes'],history+[name])
            
def ResolveModeStrs(options,mode,curr):
	for name,d in curr.items():
		if '/' in name:
			print(f"{ERROR()}Mode name cannot contain '/' ({MODE()}{ModeStr(mode)}{ERROR()})!")
			ErrorExit()
			
		if type(d) is str:
			var = d[1:]
			rep = GetModeVar(options,mode,var)
			if rep is None:
				print(f"{ERROR()}Cannot resolve variable mode '{var}'!")
				ErrorExit()
			curr[name] = copy.deepcopy(rep)
			d = curr[name]
		if type(d) is not dict:
			print(f"{ERROR()}Type of mode {MODE()}{ModeStr(mode)}{ERROR()} must be dict!")
			ErrorExit()
			
		if 'modes' in d:
			if type(d['modes']) is str and d['modes'][0]=='%':
				var = d['modes'][1:]
				rep = GetModeVar(options,mode,var)
				if rep is None:
					print(f"{ERROR()}Cannot resolve 'modes' variable '{var}'!")
					ErrorExit()
				d['modes'] = copy.deepcopy(rep)
				
			if type(d['modes']) is not dict:
				print(f"{ERROR()}Type of 'modes' in mode {MODE()}{ModeStr(mode)}{ERROR()} must be dict!")
				ErrorExit()
			
			ResolveModeStrs(options,mode+[name],d['modes'])

def GetOptionsFromFile(file):
    if not os.path.exists(f".{os.path.sep}{file}"):
        if file=='Builderfile':
            file = 'builder.json'
            if not os.path.exists(f".{os.path.sep}{file}"):
                print(f"{ERROR()}No Builderfile found!")
                ErrorExit()
        else:
            print(f"{ERROR()}No {file} file found!")
            ErrorExit()

    s = ''
    with open(file,'r') as f:
        s = f.read()
    
    try:
        op = json.JSONDecoder().decode(s)
    except json.decoder.JSONDecodeError as e:
        print(f'{ERROR()}JSON Decode Error ({file}):')
        print('\t'+str(e))
        ErrorExit()

    error = False

    if 'modes' not in op:
        print(f"{ERROR()}Builder file must specify 'modes'!")
        ErrorExit()
    elif type(op['modes']) is str and op['modes'][0]=='%':
        if op['modes'][1:] not in op:
            print(f"{ERROR()}Variable '{op['modes'][1:]}' not found!")
            ErrorExit()
        op['modes'] = op[op['modes'][1:]]
        
    if type(op['modes']) is not dict:
        print(f"{ERROR()}Expected 'modes' to be of type dict!")
        ErrorExit()
    elif len(op['modes'])==0:
        print(f"{ERROR()}Builder file must specify at least one mode in 'modes'!")
        ErrorExit()
        
    if 'defaultMode' in op and '/' in op['defaultMode']:
        print(f"{ERROR()}Default mode name cannot contain '/'!")
        ErrorExit()

    modes = op['modes']
    
    ResolveModeStrs(op,[],op['modes'])
    CreateSubModeDicts(modes)

    defaults = [('compileCmd',''),('linkCmd',''),('outputName','a'),
            ('defaultMode',list(op['modes'].keys())[0]),('sourceExt',['c','cpp','c++']),
            ('headerExt',['h','hpp','h++']),('objectExt','o'),('sourceDir','.'),('includeDir',''),
            ('objectDir','.'),('outputDir','.'),('preCmds',[]),('postCmds',[])]

    SetDefaults(op,defaults)
    
    FixDirs(op)

    if error:
        ErrorExit()

    return op

def main():    
    global noColor
    name = 'builder'
    builderVersion = '0.1.1'
    
    os.system('')

    parser = argparse.ArgumentParser(prog='builder',description="Only builds what needs to be built.")
    group = parser.add_mutually_exclusive_group()
    actions = parser.add_mutually_exclusive_group()
    parser.add_argument("mode",default='',help="specify the build modes to run",nargs='*')
    parser.add_argument("-b",metavar='FILE',default='Builderfile',help="specify name of builder file to use (default Builderfile, builder.json)")
    actions.add_argument("-c","--clean",help="remove all object and output files",action="store_true")
    actions.add_argument("--stats",action="store_true",help="print stats about the project")
    actions.add_argument("-l","--list",action="store_true",help="print all available modes")
    parser.add_argument("-s","--single",action="store_true",help="run single-threaded")
    group.add_argument("-v","--verbose",help="print more info for debugging",action="store_true")
    group.add_argument("-q","--quiet",help="silence builder output",action="store_true")
    parser.add_argument("--log",metavar="FILE",default="",help="write output to the specified log file")
    parser.add_argument("--nocolor",help="disables output of color escape sequences",action="store_true")
    parser.add_argument("--version",action="store_true",help='show program\'s version number and exit')
    args = parser.parse_args()

    if args.nocolor or not sys.stdout.isatty():
        noColor = True

    builderLog = ''

    if args.log:
        noColor = True
        sys.stdout.close()
        f = open(args.log,'w')
        sys.stdout = f
        sys.stderr = f
    
    if args.version:
        print(f'{TextColor(WHITE,1)}{name} {MODE()}{builderVersion}{RESET()}')
        quit()
        
    modes = [[]]
    if args.mode!='':
        modes = ParseArgModes(args.mode)
        
    builderFile = args.b
    options = GetOptionsFromFile(builderFile)
    b = Builder(options)
    
    if args.verbose:
        b.debug = True
    
    if args.quiet:
        b.quiet = True
    
    if args.single:
        b.single = True

    if args.list:
        b.List(modes[0])
        quit()

    if args.stats:
        b.Stats(modes[0])
        quit()

    if args.clean:
        for mode in modes:
            b.Clean(mode)
    else:
        for mode in modes:
            b.Build(mode)

    print(RESET(),end='')


if __name__=='__main__':
    try:
        main()
    except KeyboardInterrupt:
        print()
        quit(1)
