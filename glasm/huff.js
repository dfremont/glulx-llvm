"use strict";
if(!RegExp.quote) RegExp.quote=x=>String(x).replace(/[-\\^$*+?.()|[\]{}]/g,"\\$&");
const Huff=module.exports;
const usage=new Int32Array(0x4000);
const strings=Object.create(null);
Huff.add=(txt,inodes)=>{
  strings[txt]=true;
  inodes.forEach((v,k)=>{
    if(typeof(v)==="string") txt=txt.replace(new RegExp(RegExp.quote(v),"g"),String.fromCharCode(k+256));
  });
  for(let i=0;i<txt.length;i++) usage[txt.charCodeAt(i)]++;
  usage[0]++;
};
let root;
const encode=(txt,inodes)=>{
  let n=0;
  inodes.forEach((v,k)=>{
    if(typeof(v)==="string") txt=txt.replace(new RegExp(RegExp.quote(v),"g"),String.fromCharCode(k+256));
  });
  txt+="\0";
  for(let i=0;i<txt.length;i++) {
    let v=txt.charCodeAt(i);
    while(usage[v]) n++,v=usage[v]&0x7FFF;
  }
  let b=Buffer.alloc((n+7)>>3);
  n=0;
  for(let i=0;i<txt.length;i++) {
    let v=txt.charCodeAt(i);
    let e=[];
    while(usage[v]) e.unshift(usage[v]>>15),v=usage[v]&0x7FFF;
    e.forEach(x=>{
      b[n>>3]|=x<<(n&7);
      n++;
    });
  }
  return b;
};
Huff.calc=inodes=>{
  if(inodes.length>0x1F00) throw "Too many !fword and/or !inode definitions";
  // Figure out Huffman tree
  let q1=[];
  let q2=[];
  let v=0x2000;
  let n1,n2;
  for(let i=0;i<0x2000;i++) if(usage[i]) q1.push(i);
  q1.sort((x,y)=>usage[x]-usage[y]);
  while(q1.length+q2.length>1) {
    if(!q1.length) {
      [n1,n2]=[q2.shift(),q2.shift()];
    } else if(!q2.length) {
      [n1,n2]=[q1.shift(),q1.shift()];
    } else {
      if(usage[q2[0]]<usage[q1[0]]) n1=q2.shift(); else n1=q1.shift();
      if(!q1.length) n2=q2.shift();
      else if(!q2.length) n2=q1.shift();
      else if(usage[q2[0]]<usage[q1[0]]) n2=q2.shift(); else n2=q1.shift();
    }
    usage[v]=usage[n1]+usage[n2];
    usage[n1]=v;
    usage[n2]=v|0x8000;
    q2.push(v++);
  }
  usage[root=q2[0]]=0;
  // Encode strings
  for(let t in strings) strings[t]=encode(t,inodes);
};
Huff.tree=(mem,cur,inodes)=>{
  let len=12+9;
  let nn=1;
  let addr={};
  for(let i=0;i<0x4000;i++) if(usage[i]) {
    nn++;
    addr[i]=cur+len;
    if(!i) {
      // String terminator node
      len++;
    } else if(i<256) {
      // Single character node
      len+=2;
    } else if(i<0x2000) {
      // String or indirect node
      let v=inodes[i-256];
      if(typeof(v)==="string") len+=v.length+2;
      else len+=4*v.length-3;
    } else {
      // Branch node
      len+=9;
    }
  }
  if(!mem) return cur+len;
  addr[root]=cur+12;
  mem.writeInt32BE(len,cur); cur+=4;
  mem.writeInt32BE(nn,cur); cur+=4;
  mem.writeInt32BE(addr[root],cur); cur+=4;
  mem[cur]=0x00; // root node
  cur+=9;
  for(let i=0;i<0x4000;i++) if(usage[i]) {
    if(usage[i]&0x8000) mem.writeInt32BE(cur,addr[usage[i]&0x7FFF]+5);
    else mem.writeInt32BE(cur,addr[usage[i]]+1);
    if(!i) {
      mem[cur++]=0x01;
    } else if(i<256) {
      mem[cur++]=0x02;
      mem[cur++]=i;
    } else if(i<0x2000) {
      let v=inodes[i-256];
      if(typeof(v)==="string") {
        mem[cur++]=0x03;
        cur+=mem.write(v,cur,v.length,"binary");
        mem[cur++]=0;
      } else {
        mem[cur++]=v[0];
        let w=1;
        while(w<v.length) {
          mem.writeInt32BE(v[w++]|0,cur);
          cur+=4;
        }
      }
    } else {
      mem[cur]=0x00;
      cur+=9;
    }
  }
  return cur;
};
Huff.string=(txt,mem,cur)=>{
  if(mem) mem[cur]=0xE1,strings[txt].copy(mem,cur+1);
  return cur+strings[txt].length+1;
};
Huff.dump=()=>{
  const fs=require("fs");
  Object.keys(strings).forEach(x=>fs.writeSync(1,x+"\0",null,"utf8"));
}
