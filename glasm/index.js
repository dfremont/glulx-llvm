// Glasm -- a Glulx assembler in JavaScript
// This program is public domain.
"use strict";
const fs=require("fs");
const files=Object.create(null);
const extensions=Object.create(null);
const readF=n=>{
  try {
    return files[n]=files[n]||fs.readFileSync(n,"binary").replace(/\r/g,"").split("\n").map(x=>x.trim());
  } catch(e) {
    throw "Unable to read file";
  }
};
const options={};
if(process.argv[2]) process.argv[2].split("").forEach(x=>(options[x]=true));
let mem;
let phase=0;
let changed=true;
let cur=36;
let skipping=0;
let defining=null;
let munique=0;
let ramstart=0xFFFFFFFF;
let bsslength=0;
const deferred=[];
const names=Object.create(null);
const fwnames=new Array(10);
const decodeUTF8char=x=>Buffer.from(x,"binary").toString("utf8").codePointAt();
const decodeUTF8string=(x,a)=>{
  let b=[...Buffer.from(x,"binary").toString("utf8")];
  for(let y of b) mem.writeInt32BE(y.codePointAt(),a),a+=4;
  return b.length;
};
const macstack=[];
const cond=Object.assign(Object.create(null),{
  def: a=>(names[a]!==undefined),
  int: a=>(typeof(a)==="number"),
  js: a=>(a),
  local: a=>(Array.isArray(a) && a[0]=="$"),
  mem: a=>(Array.isArray(a) && a[0]=="@"),
  neg: a=>(typeof(a)==="number" && a<0),
  nz: a=>(typeof(a)==="number" && a),
  pos: a=>(typeof(a)==="number" && a>0),
  str: a=>(typeof(a)==="string"),
  undef: a=>(names[a]===undefined),
  z: a=>(typeof(a)==="number" && !a),
});
let Huff;
const inodes=[];
let filterF=null;
const bang={
  align: ([a])=>{
    if(typeof(a)!=="number") throw "Argument must be a number";
    if(a<1 || (a&(a-1))) throw "Argument must be a power of two";
    if(cur&(a-1)) cur=(cur|(a-1))+1;
    return cur;
  },
  alignbss: ([a])=>{
    if(typeof(a)!=="number") throw "Argument must be a number";
    if(a<1 || (a&(a-1))) throw "Argument must be a power of two";
    if(bsslength&(a-1)) bsslength=(cur|(a-1))+1;
    return cur+bsslength;
  },
  allot: ([a])=>{
    if(typeof(a)!=="number" || a<0) throw "Argument must be a nonnegative number";
    bsslength+=a;
    return cur+bsslength-a;
  },
  append: ([a,b],m)=>{
    if(!m) throw "Not valid outside of a macro";
    if(typeof(a)!=="number" || a<1 || a>9) throw "Argument must be a number 1 to 9";
    if(typeof(b)!=="number" && typeof(b)!=="string") throw "Argument must be a number or string";
    if(m[a-1]===undefined) throw "Macro parameter not defined";
    m[a-1]+=","+b;
  },
  assert: a=>{
    bang.if(a);
    if(skipping) throw "Assertion failed";
  },
  at: ([a])=>{
    if(typeof(a)!=="number" || a<36) throw "Argument must be at least 36";
    cur=a;
  },
  binary: ([a])=>{
    if(typeof(a)!=="string") throw "Argument must be a filename";
    try {
      let s=fs.statSync(a);
      if(!s) throw "Unable to stat file: "+a;
      if(mem) {
        let fd=fs.openSync(a,"r");
        fs.readSync(fd,mem,cur,s.size,0);
        fs.closeSync(fd);
      }
      cur+=s.size;
    } catch(e) {
      if(typeof(e)==="string") throw e;
      throw "Unable to open file: "+a;
    }
  },
  bss: ()=>{
    if(cur&255) cur=(cur|255)+1;
    if(ramstart>cur) ramstart=cur;
    bsslength=0;
    return cur;
  },
  data: a=>{
    for(let v of a) {
      if(typeof(v)==="number") {
        if(mem) mem.writeInt32BE(v,cur);
        cur+=4;
      } else if(typeof(v)==="string") {
        if(mem) {
          cur+=decodeUTF8string(v,cur)<<2;
        } else {
          let i;
          for(i=0;i<v.length;i++) if(v.charCodeAt(i)<0x80 || v.charCodeAt(i)>0xBF) cur+=4;
        }
      } else {
        throw "Data item must be a number or string";
      }
    }
  },
  datab: a=>{
    for(let v of a) {
      if(typeof(v)==="number") {
        if(mem) mem[cur]=v&255;
        cur++;
      } else if(typeof(v)==="string") {
        if(mem) mem.write(v,cur,v.length,"binary");
        cur+=v.length;
      } else {
        throw "Data item must be a number or string";
      }
    }
  },
  datas: a=>{
    for(let v of a) {
      if(typeof(v)==="number") {
        if(mem) mem.writeUInt16BE(v&0xFFFF,cur);
        cur+=2;
      } else {
        throw "Data item must be a number";
      }
    }
  },
  define: ([a,c])=>{
    if(typeof(a)!=="string" || typeof(c)!=="string") throw "Argument must be a string";
    if(names[a]!==undefined && !names[a].Macro) throw "Can't redefine name";
    names[a]={Macro:[c.trim()]};
  },
  echo: a=>{
    console.error(JSON.stringify(a));
  },
  else: ()=>{
    skipping=1;
  },
  error: a=>{
    throw "User error: "+a.join(" ");
  },
  extension: ([a,...b])=>{
    if(!options.x) throw "Extensions are not enabled";
    if(typeof(a)!=="string") throw "Argument must be a string";
    if(a[0]==":") a=__dirname+"/"+a.slice(1)+".ext.js";
    if(!global.Glasm) global.Glasm={
      addPhase: x=>{
        if(phase<x) changed=true;
      },
      address: ()=>cur,
      appendLine: x=>files[0].push(x),
      data: x=>{
        let i=cur;
        if(!x) return i;
        if(!Buffer.isBuffer(x)) throw new TypeError("Value to add must be a Buffer instance");
        cur+=x.length;
        if(mem) {
          if(cur>mem.length) throw new RangeError("Attempt to write beyond end of memory");
          x.copy(mem,i);
        }
        return i;
      },
      defer: deferred,
      define: define,
      defineCommand: (n,f)=>{
        if(phase) throw new Error("Cannot use Glasm.defineCommand()");
        if(bang[n] && bang[n]!==f) throw new Error("Command !"+n+" already defined");
        return bang[n]=f;
      },
      filter: x=>(filterF=x),
      include: x=>{
        if(typeof(x)==="string") x=[x];
        files["<extension>"]=x;
        return parseF("<extension>");
      },
      memory: ()=>mem,
      phase: ()=>phase,
    };
    if(!extensions[a]) extensions[a]=require(require("path").resolve(a));
    let i=extensions[a];
    if(typeof(i)==="function") i=i(b);
    if(Array.isArray(i)) global.Glasm.include(i);
    if(Buffer.isBuffer(i)) global.Glasm.data(i);
    if(typeof(i)==="number") cur+=i;
  },
  fi: ()=>{},
  float: ([a])=>{
    if(typeof(a)!=="string") throw "Argument must be a string";
    if(mem) mem.writeFloatBE(parseFloat(a),cur);
    cur+=4;
  },
  fword: a=>{
    a=a.map(x=>{
      if(typeof(x)==="string") return x;
      if(typeof(x)==="number" && x>0 && x<256) return String.fromCharCode(x);
      throw "Data item must be a number in range 1 to 255 or a string";
    }).join("");
    if(a.length<2) throw "Length of fword must be at least 2";
    let i=inodes.indexOf(a);
    if(i!=-1) return i+256;
    inodes.push(a);
    return inodes.length+255;
  },
  huffstring: a=>{
    a=a.map(x=>{
      if(typeof(x)==="string") return x;
      if(typeof(x)==="number" && x>0) return String.fromCharCode(x);
      throw "Data item must be a positive inode number or a string";
    }).join("");
    if(phase==1) {
      if(!Huff) throw "Found !huffstring without !hufftree";
      Huff.add(a,inodes);
    } else if(phase>1) {
      cur=Huff.string(a,mem,cur);
    }
    if(phase<=1) cur+=3; // estimation
  },
  hufftree: ()=>{
    if(Huff && !phase) throw "Found multiple !hufftree commands";
    if(!Huff) {
      if(phase) throw "Deferred !hufftree";
      Huff=require("./huff.js");
    }
    if(mem) mem.writeUInt32BE(cur,28);
    cur=Huff.tree(mem,cur,inodes);
    if(phase<2) changed=true;
  },
  if: ([a,b])=>{
    if(b===undefined) b="js";
    if(typeof(b)!=="string" || !cond[b]) throw "Invalid condition";
    skipping=cond[b](a)?0:1;
  },
  include: ([a])=>{
    if(typeof(a)!=="string") throw "Argument must be a string";
    if(a[0]==":") a=__dirname+"/"+a.slice(1)+".inc";
    return parseF(a);
  },
  inode: ([a,...b],mmm,lbl)=>{
    if(!lbl) throw "Label required";
    if(typeof(a)==="number") {
      a=[0,a];
    } else if(Array.isArray(a) && a[0]=="@") {
      a=[1,a];
    } else {
      throw "Argument must be an address, either direct or indirect";
    }
    if(b.length) {
      if(b.some(x=>typeof(x)!=="number")) throw "Argument must be a number";
      b.unshift(a[0]+10,a[1],b.length);
    } else {
      b.unshift(a[0]+8,a[1]);
    }
    if(!names[lbl]) {
      inodes.push(b);
      return inodes.length+255;
    }
    if(typeof(names[lbl])!=="number" || names[lbl]<256) throw "Misdefined label";
    inodes[names[lbl]-256]=b;
    return names[lbl];
  },
  is: ([a])=>{
    return a;
  },
  isf: ([a])=>{
    let z=new ArrayBuffer(4);
    let y=new Float32Array(z);
    let x=new Int32Array(z);
    if(typeof(a)!=="string") throw "Argument must be a string";
    y[0]=parseFloat(a);
    return x[0];
  },
  lfunc: ([a])=>{
    if(typeof(a)!=="number" || a<0 || a>255) throw "Argument must be a number in range 0 to 255";
    if(mem) {
      mem[cur++]=0xC1;
      if(a) {
        mem[cur++]=4;
        mem[cur++]=a;
      }
      mem[cur++]=0;
      mem[cur++]=0;
    } else {
      cur+=a?5:3;
    }
  },
  macro: ([a])=>{
    if(typeof(a)!=="string") throw "Argument must be a string";
    if(names[a]!==undefined && !names[a].Macro) throw "Can't redefine name";
    names[a]={Macro:defining=[]};
  },
  main: a=>{
    if(mem) mem.writeUInt32BE(cur,24);
    return bang.sfunc(a);
  },
  mpush: a=>{
    macstack.push(...a);
  },
  op: ([a,b,...c])=>{
    let h=cur;
    let n=0;
    let i,v;
    if(typeof(a)!=="number" || a<0 || a>0x0FFFFFFF) throw "Opcode number must be a number in range 0 to 0x0FFFFFFF";
    if(typeof(b)!=="number" || b<0) throw "Number of operands must be at least 0";
    if(c.length!=b) throw "Incorrect number of operands";
    h+=(a>0x3FFF?4:a>0x7F?2:1);
    if(mem) {
      if(a>0x3FFF) mem.writeUInt32BE(a+0xC0000000,cur);
      else if(a>0x7F) mem.writeUInt16BE(a+0x8000,cur);
      else mem[cur]=a;
    }
    for(i=0;i<c.length;i++) {
      if(!(i&1)) {
        if(mem) mem[h]=0;
        h++;
      }
      v=c[i];
      if(typeof(v)==="number") {
        n+=(v==0?0:v>=-0x80&&v<0x80?1:v>=-0x8000&&v<0x8000?2:4);
        if(mem) mem[h-1]|=(v==0?0:v>=-0x80&&v<0x80?1:v>=-0x8000&&v<0x8000?2:3)<<(i&1?4:0);
      } else if(Array.isArray(v)) {
        if(v.length==1 && v[0]=="$") {
          if(mem) mem[h-1]|=8<<(i&1?4:0);
        } else if(v.length==1) {
          throw "Invalid operand";
        } else if(v[0]=="@") {
          if(v[1]>=ramstart) {
            v=v[1]-ramstart;
            n+=(v<0x100?1:v<0x10000?2:4);
            if(mem) mem[h-1]|=(v<0x100?13:v<0x10000?14:15)<<(i&1?4:0);
          } else {
            v=v[1];
            n+=(v<0x100?1:v<0x10000?2:4);
            if(mem) mem[h-1]|=(v<0x100?5:v<0x10000?6:7)<<(i&1?4:0);
          }
        } else if(v[0]=="/") {
          v=v[1]-h-n;
          n+=(v>=-0x78&&v<0x78?1:v>=-0x7FF8&&v<0x7FF8?2:4);
          if(mem) mem[h-1]|=(v>=-0x78&&v<0x78?1:v>=-0x7FF8&&v<0x7FF8?2:3)<<(i&1?4:0);
        } else if(v[0]=="$") {
          v=v[1]<<2;
          n+=(v<0x100?1:v<0x10000?2:4);
          if(mem) mem[h-1]|=(v<0x100?9:v<0x10000?10:11)<<(i&1?4:0);
        }
      } else {
        throw "Type mismatch";
      }
    }
    cur=h+n;
    if(mem) for(i=0;i<c.length;i++) {
      v=c[i];
      if(typeof(v)==="number") {
        if(v) {
          n=(v>=-0x80&&v<0x80?1:v>=-0x8000&&v<0x8000?2:4);
          mem[n==1?"writeInt8":n==2?"writeInt16BE":"writeInt32BE"](v,h);
          h+=n;
        }
      } else if(v[0]=="@") {
        v=v[1];
        if(v>=ramstart) v-=ramstart;
        n=(v<0x100?1:v<0x10000?2:4);
        mem[n==1?"writeUInt8":n==2?"writeUInt16BE":"writeUInt32BE"](v,h);
        h+=n;
      } else if(v[0]=="/") {
        v=v[1]-h;
        n=(v>=-0x78&&v<0x78?1:v>=-0x7FF8&&v<0x7FF8?2:4);
        v-=n-2;
        mem[n==1?"writeInt8":n==2?"writeInt16BE":"writeInt32BE"](v,h);
        h+=n;
      } else if(v.length==2) { // $
        v=v[1]<<2;
        n=(v<0x100?1:v<0x10000?2:4);
        mem[n==1?"writeUInt8":n==2?"writeUInt16BE":"writeUInt32BE"](v,h);
        h+=n;
      }
    }
  },
  push: a=>{
    for(let v of a) bang.op([0x40,2,v,["$"]]);
  },
  pushr: a=>{
    let i;
    for(i=a.length-1;i>=0;i--) bang.op([0x40,2,a[i],["$"]]);
  },
  ram: ([a])=>{
    if(cur&255) cur=(cur|255)+1;
    if(typeof(a)==="number" && phase && phase>=a && cur<ramstart) cur=ramstart;
    return ramstart=cur;
  },
  relop: ([a,b,...c])=>{
    let x=c[c.length-1];
    if(typeof(x)!=="number" || x<0) throw "Branch offset must be a nonnegative number";
    if(x>1) x=["/",x];
    return bang.op([a,b,...c.slice(0,-1),x]);
  },
  set: ([a,b],m)=>{
    if(!m) throw "Not valid outside of a macro";
    if(typeof(a)!=="number" || a<1 || a>9) throw "Argument must be a number 1 to 9";
    if(typeof(b)!=="number" && typeof(b)!=="string") throw "Argument must be a number or string";
    m[a-1]=b;
  },
  setq: ([a,b],m)=>{
    if(!m) throw "Not valid outside of a macro";
    if(typeof(a)!=="number" || a<1 || a>9) throw "Argument must be a number 1 to 9";
    if(typeof(b)!=="number" && typeof(b)!=="string") throw "Argument must be a number or string";
    if(typeof(b)==="string") m[a-1]="\""+b.replace(/"/g,"\"\"")+"\"";
    else m[a-1]=String(b);
  },
  sfunc: ([a])=>{
    if(typeof(a)!=="number" || a<0 || a>255) throw "Argument must be a number in range 0 to 255";
    if(mem) {
      mem[cur++]=0xC0;
      if(a) {
        mem[cur++]=4;
        mem[cur++]=a;
      }
      mem[cur++]=0;
      mem[cur++]=0;
    } else {
      cur+=a?5:3;
    }
  },
  shift: (a,m)=>{
    return m.shift();
  },
  stack: ([a])=>{
    if(!mem) return;
    if(typeof(a)!=="number" || a<1) throw "Stack size must be a positive number";
    a<<=2;
    if(a&255) a=(a|255)+1;
    mem.writeUInt32BE(a,20);
  },
  string: a=>{
    if(mem) mem[cur]=0xE0;
    cur++;
    bang.datab(a);
    if(mem) mem[cur]=0;
    cur++;
  },
  unchanged: ()=>{
    changed=false;
  },
  unistring: a=>{
    if(mem) mem[cur]=0xE2;
    cur++;
    bang.datab([0,0,0]); // padding
    bang.data(a);
    if(mem) mem[cur]=0;
    cur++;
  },
  zero: ([a])=>{
    if(typeof(a)!=="number" || a<0) throw "Argument must be a nonnegative number";
    let i;
    for(i=0;i<a;++i) {
      if(mem) mem[cur]=0;
        cur++;
    }
  },
};
const replaceMacroArgs=(li,m)=>li.replace(/\\./g,x=>{
  switch(x[1]) {
    case "1": return m[0];
    case "2": return m[1];
    case "3": return m[2];
    case "4": return m[3];
    case "5": return m[4];
    case "6": return m[5];
    case "7": return m[6];
    case "8": return m[7];
    case "9": return m[8];
    case "#": return m.length;
    case "\\": return "\\";
    case "*": return m.join(",");
    case "@": return m.Unique;
    case "_": return " ";
    case "%": return macstack.pop();
    case "?": return macstack.length;
    default: throw "Invalid token "+x;
  }
});
const parseV=s=>{
  let n=0;
  s="("+s.replace(/'[\x80-\xFF]+'|'[^]'|[0-9A-Za-z_.?]+|!|[ \t]+/g,t=>{
    if(t.length==3 && t[0]=="'") {
      return "("+t.charCodeAt(1)+")";
    } else if(t[0]=="'") {
      return "("+decodeUTF8char(t.slice(1,-1))+")";
    } else if(/^(0|[1-9][0-9]*)$/.test(t)) {
      return "("+t+")";
    } else if(/^[0-9][BFbf]$/.test(t)) {
      if(t[1]=="b" || t[1]=="B") n=-1; else n=0;
      t="H "+t[0]+(fwnames[t[0]]+n);
      if(phase && typeof(names[t])!=="number") throw "Misdefined name: "+t;
      return "("+(names[t]|0)+")";
    } else if(/^0[Xx][0-9A-Fa-f]+$/.test(t)) {
      return "("+(parseInt(t)|0)+")";
    } else if(t=="!") {
      return "("+cur+")";
    } else if(t[0]==" " || t[0]=="\t") {
      return "";
    } else {
      if(phase && typeof(names[t])!=="number") throw (names[t]===undefined?"Un":"Mis")+"defined name: "+t;
      return "("+(names[t]|0)+")";
    }
  })+")";
  if(/[^0-9/+*^|&%()~-]|[^0-9()]{2}|\)[0-9(]|\([^0-9(~-]/.test(s)) throw "Syntax error in expression";
  try {
    return eval(s)|0;
  } catch(e) {
    throw "Syntax error in expression";
  }
};
const parseArg=s=>{
  const a=[];
  let m;
  s=s.trim();
  if(s[0]!=",") s=","+s;
  while(m=/^[ \t]*,[ \t]*("(?:\\"|""|[^"])*"|(?:[^,;="']|'[^]'|'[\xC0-\xFF][\x80-\xBF]+')*)/.exec(s)) {
    if(m[1][0]=="\"") {
      let q=m[1].slice(1,-1).replace(/""/g,"\"");
      const unescape=(match, p1, offset, string)=>{
        switch(p1) {
          case "\\": return "\\";
          case "\"": return "\"";
          case "b": return "\b";
          case "f": return "\f";
          case "n": return "\n";
          case "r": return "\r";
          case "t": return "\t";
          default: return String.fromCharCode(parseInt(p1,8));
        }
      }
      a.push(q.replace(/\\([\\\"bfnrt]|[0-7][0-7][0-7])/g,unescape));
    } else {
      let q="";
      let v=m[1];
      if(names[v]!==undefined && !names[v].Macro) {
        a.push(names[v]);
      } else if(/^-?[0-9]+$/.test(v)) {
        a.push(parseInt(v,10));
      } else {
        if(v[0]=="@" || v[0]=="$") q=v[0],v=v.slice(1);
        // @ address
        // $ local or stack
        v=v.trim();
        if(!v && !q) {
          if(!a.length) {
            s=s.slice(m[0].length);
            break;
          }
          throw "Empty argument";
        }
        a.push(v?q?[q,parseV(v)]:parseV(v):[q]);
      }
    }
    s=s.slice(m[0].length);
  }
  s=s.trim();
  if(s[0]=="=") {
    a.push(s.slice(1));
    return a;
  } else if(!s || s[0]==";") {
    return a;
  } else {
    throw "Syntax error";
  }
};
const parseMacArg=s=>{
  const a=[];
  let m;
  s=s.trim();
  if(s[0]!=",") s=","+s;
  while(m=/^[ \t]*,((?:'[^]'|'[\xC0-\xFF][\x80-\xBF]+'|[^,;="']|"[^"]*")*)/.exec(s)) {
    a.push(m[1].trim());
    s=s.slice(m[0].length);
  }
  s=s.trim();
  if(s[0]=="=") {
    a.push("\""+s.slice(1).replace(/"/g,"\"\"")+"\"");
    return a;
  } else if(!s || s[0]==";") {
    return a;
  } else {
    throw "Syntax error";
  }
};
const define=(k,v)=>{
  if(names[k]!==v) {
    if(names[k]===undefined) changed=true;
    else if(Array.isArray(v) && JSON.stringify(names[k])!=JSON.stringify(v)) changed=true;
    else if(typeof(v)!=="object") changed=true;
    if(options.D) console.error([k,v]);
    if(names[k]!==undefined && names[k].Macro && !v.Macro) throw "Cannot redefine name";
  }
  names[k]=v;
};
const parseF=(filename,macroargs)=>{
  let lnum=0;
  try {
    let lines=readF(filename);
    let li,li0,lbl,c;
    if(options.D && !macroargs) console.error("Begin: "+filename);
    for(;;) {
      if(li0) {
        li=li0;
        li0=undefined;
      } else {
        lbl=null;
        li=lines[lnum++];
        if(lnum>lines.length) break;
        if(macroargs) li=replaceMacroArgs(li,macroargs);
      }
      if(filterF) li=filterF(li);
      if(!li || li[0]==";") continue;
      if(defining) {
        li=li.trim();
        if(li=="!endm") {
          defining=null;
        } else {
          defining.push(li);
        }
        continue;
      }
      li=/^([^ \t]+)(?:[ \t]+([^]*))?$/.exec(li);
      if(!li) throw "Syntax error";
      if(!li[2]) li[2]="";
      if(skipping) {
        if(li[1]=="!fi") --skipping;
        else if(li[1]=="!else" && skipping==1) skipping=0;
        else if(li[1]=="!if") ++skipping;
        continue;
      }
      if(li[1][0]=="<") {
        if(phase) continue;
        li[1]=li[1].slice(1);
      } else if(li[1][0]==">") {
        if(!phase) continue;
        li[1]=li[1].slice(1);
      }
      if(li[1][0]=="!") {
        if(li[1]=="!loop") {
          bang.if(parseArg(li[2]),macroargs);
          if(!skipping) lnum=0;
          skipping=0;
          continue;
        } else if(li[1]=="!ret") {
          return parseArg(li[2])[0];
        } else if(li[1]=="!deferred") {
          li0=li[2];
          lbl=deferred.pop();
          continue;
        }
        if(!bang[li[1].slice(1)]) throw "Invalid directive: "+li[0];
        c=cur;
        let x=bang[li[1].slice(1)](parseArg(li[2]),macroargs,lbl);
        if(x===deferred) {
          deferred.push(lbl);
          lbl=null;
        }
        if(lbl && x!==undefined) define(lbl,x);
        if(lbl && x===undefined && names["N "+lbl]===phase) define(lbl,c);
      } else if(li[1][0]==":") {
        lbl=li[1].slice(1);
        if(names["N "+lbl]===phase) throw "Duplicate definition of '"+lbl+"'";
        names["N "+lbl]=phase;
        li0=li[2];
        if(!li0) define(lbl,cur);
      } else if(li[1].length==2 && (li[1][1]=="H" || li[1][1]=="h") && li[1][0]>="0" && li[1][0]<="9") {
        lbl="H "+li[1][0]+fwnames[li[1][0]]++;
        names["N "+lbl]=phase;
        li0=li[2];
        if(!li0) define(lbl,cur);
      } else if(typeof(names[li[1]])==="object" && names[li[1]].Macro) {
        files["<macro>"]=names[li[1]].Macro;
        let ma=parseMacArg(li[2]);
        ma.Unique=munique++;
        c=cur;
        let x=parseF("<macro>",ma);
        if(lbl && x!==undefined) define(lbl,x);
        if(lbl && x===undefined && names["N "+lbl]===phase) define(lbl,c);
      } else {
        throw "Undefined macro: "+li[1];
      }
    }
    if(options.D && !macroargs) console.error("End: "+filename);
  } catch(e) {
    if(typeof(e)!=="string") throw e;
    throw e+"\n  on line "+lnum+" of "+(filename||"<stdin>");
  }
};
const checksum=()=>{
  let v=0;
  let i;
  for(i=0;i<mem.length;i+=4) v=(v+mem.readInt32BE(i))|0;
  return v;
};
try {
  process.argv.slice(3).forEach(x=>{
    let m;
    if(m=/^([^ =]+)=(-?[0-9]+|-?0[Xx][0-9a-fA-F]+)$/.exec(x)) {
      names[m[1]]=parseInt(m[2])|0;
    } else if(m=/^([^ =]+)==([^]*)$/.exec(x)) {
      names[m[1]]=m[2];
    } else if(x) {
      throw "Invalid command-line argument";
    }
  });
  while(changed) {
    if(phase>255) {
      console.error("Error:\n  Too many phases");
      process.exit(1);
    }
    console.error("Phase "+phase);
    changed=false;
    cur=36;
    fwnames.fill(0);
    munique=0;
    parseF(0);
    define("!TOTAL!",cur);
    if(skipping) {
      console.error("Error:\n  Input ended during skipping");
      process.exit(1);
    }
    if(Huff && phase==1) Huff.calc(inodes);
    ++phase;
  }
  console.error("Last phase ("+cur+" bytes)");
  mem=Buffer.alloc((cur+255)&~255);
  mem.writeInt32BE(0x30102,4);
  cur=36;
  fwnames.fill(0);
  munique=0;
  parseF(0);
  if(changed) throw "Unexpected changes after everything seems to be stable";
  mem.write("Glul",0);
  if(cur&255) cur=(cur|255)+1;
  if(ramstart>cur) ramstart=cur;
  if(bsslength&255) bsslength=(bsslength|255)+1;
  mem.writeUInt32BE(ramstart,8);
  mem.writeUInt32BE(cur,12);
  mem.writeUInt32BE(cur+bsslength,16);
  mem.writeInt32BE(0,32);
  mem.writeInt32BE(checksum(),32);
} catch(e) {
  if(typeof(e)!=="string") throw e;
  console.error("Error:\n  %s",e);
  process.exit(1);
}
if(options.n) {
  mem=null;
  for(let x in names) {
    if(x[1]!==" " && names["N "+x]) {
      if(Array.isArray(names[x])) console.log(x+"="+names[x].join(""));
      else if(typeof(names[x])==="number") console.log(x+"="+names[x]);
    }
  }
} else if(options.h) {
  if(Huff) Huff.dump();
} else {
  fs.writeSync(1,mem,0,mem.length);
}
