#include "nxsbuild.h"
#include <set>
#include <map>

using namespace nxs;
using namespace vcg;
using namespace std;

void nxs::RemapVertices(Crude &crude,
			VertRemap &vert_remap,
			VFile<unsigned int> &face_remap,	 
			vector<unsigned int> &patch_verts) {

  for(unsigned int i = 0; i < crude.Faces(); i++) {
    Crude::Face &face = crude.GetFace(i);
    unsigned int patch = face_remap[i];
    for(int k = 0; k < 3; k++) {
      set<unsigned int> pp;
      vert_remap.GetValues(face[k], pp);
      if(!pp.count(patch)) {
	vert_remap.Insert(face[k], patch);      
	patch_verts[patch]++;
      }
    }
  }
}

void nxs::NexusAllocate(Crude &crude,
			Nexus &nexus,
			VFile<unsigned int> &face_remap,
			vector<unsigned int> &patch_faces,
			vector<unsigned int> &patch_verts) {
  
  
  nexus.index.resize(patch_faces.size());
  
  unsigned int totchunks = 0;
  //now that we know various sizes, lets allocate space
  for(unsigned int i = 0; i < nexus.index.size(); i++) {
    Nexus::Entry &entry = nexus.index[i];
    
    if(patch_faces[i] == 0 || patch_verts[i] == 0) 
      cerr << "Warning! Empty patch.\n";

    entry.patch_start = totchunks;
    entry.patch_size = Patch::ChunkSize(nexus.signature,
					patch_verts[i], patch_faces[i]);
    entry.patch_used = entry.patch_size;
    totchunks += entry.patch_size;
    entry.border_start = 0xffffffff;
    entry.nvert = patch_verts[i];
    entry.nface = 0;
    entry.error = 0;
  }

  nexus.patches.Resize(totchunks);


  //now we sort the faces into the patches (but still using absolute indexing
  //instead of relative indexing
  for(unsigned int i = 0; i < crude.face.Size(); i++) {
    Crude::Face &face = crude.face[i];
    unsigned int npatch = face_remap[i];
    
    Nexus::Entry &entry = nexus.index[npatch];

    //TODO this is slow because we have to initialize patch.
    //just get patch.start.
    Patch patch = nexus.GetPatch(npatch);
    
    Crude::Face *faces = (Crude::Face *)patch.start;

    //REMOVING degenerate faces
    if(face[0] == face[1] || face[1] == face[2] || face[0] == face[2]) {
      cerr << "Found degenerate.\n";
      continue;
    }
    faces[entry.nface] = face;
    entry.nface++;
  }
}

void nxs::NexusFill(Crude &crude,
		    Nexus &nexus,
		    VertRemap &vert_remap,
		    VFile<RemapLink> &border_remap) {
  
  
  //finally for every patch we collect the vertices
  //and fill the patch.
  //we need to remember start and size in border_remap;
  //  vector<unsigned int> border_start;
  //  vector<unsigned int> border_size;
  
  for(unsigned int i = 0; i < nexus.index.size(); i++) {
    Patch patch = nexus.GetPatch(i);
    Nexus::Entry &entry = nexus.index[i];
    
    //make a copy of faces (we need to write there :P)
    Crude::Face *faces = new Crude::Face[patch.nf];
    //Test for degenerate faces?
    memcpy(faces, (Crude::Face *)patch.start,
	   patch.nf * sizeof(Crude::Face));
    
    //collect all vertices we need.
    //TODO an hash_map would be faster?
    unsigned int count = 0;
    map<unsigned int, unsigned short> remap;
    for(unsigned int k = 0; k < patch.nf; k++) {
      Crude::Face &face = faces[k];
      
      for(int j = 0; j < 3; j++) {
        if(!remap.count(face[j])) {          
	  assert(count < patch.nv);
	  Point3f &v = crude.vert[face[j]];
          patch.VertBegin()[remap.size()] = v;
	  entry.sphere.Add(v);
          remap[face[j]] = count++;
        }
	patch.FaceBegin()[k*3 + j] = remap[face[j]];
      }
      //test for degenerate faces.
      assert(patch.FaceBegin()[k*3] != patch.FaceBegin()[k*3+1]);
      assert(patch.FaceBegin()[k*3] != patch.FaceBegin()[k*3+2]);
      assert(patch.FaceBegin()[k*3+1] != patch.FaceBegin()[k*3+2]);
    }
    assert(count == remap.size());
    assert(entry.nvert == remap.size());
    
    //record start of border:
    entry.border_start = border_remap.Size();
    
    //TODO hash_set?
    set<unsigned int> border_patches;
    map<unsigned int, unsigned short>::iterator m;
    for(m = remap.begin(); m != remap.end(); m++) {
      RemapLink link;
      link.abs_vert = (*m).first;
      link.rel_vert = (*m).second;
      
      vert_remap.GetValues(link.abs_vert, border_patches);
      assert(border_patches.size() >= 1);
      if(border_patches.size() == 1) continue; //its not a border
      
      set<unsigned int>::iterator s;
      for(s = border_patches.begin(); s != border_patches.end(); s++) {
	if((*s) == i) continue; 
	link.patch = *s;
	border_remap.PushBack(link);
      }
    }
    //and number of borders:
    entry.border_used = border_remap.Size() - entry.border_start;
    delete []faces;
  }

  //we can now update bounding sphere.
  for(unsigned int i = 0; i < nexus.index.size(); i++) 
    nexus.sphere.Add(nexus.index[i].sphere);
}

void nxs::NexusFixBorder(Nexus &nexus, 
			 VFile<RemapLink> &border_remap) {
  
  //and last convert RemapLinks into Links
  nexus.borders.Resize(border_remap.Size() * 2);
  //* 2 is to accomodate future borders

  for(unsigned int i = 0; i < nexus.index.size(); i++) {
    Nexus::Entry &local = nexus.index[i];
    local.border_start *= 2;
    local.border_size = local.border_used * 2;
  }

  for(unsigned int i = 0; i < nexus.index.size(); i++) {
    Nexus::Entry &local = nexus.index[i];

    unsigned int remap_start = local.border_start/2;
    //* 2 is to accomodate future borders


    // K is the main iterator (where we write to in nexus.borders)
    for(unsigned int k = 0;  k < local.border_used; k++) {

      
      RemapLink start_link = border_remap[k + remap_start];
      assert(start_link.rel_vert < local.nvert);

      Nexus::Entry &remote = nexus.index[start_link.patch];

      bool found = false;

      unsigned int remote_remap_start = remote.border_start/2;
      for(unsigned int j = 0; j < remote.border_used; j++) {
	
	RemapLink end_link = border_remap[j + remote_remap_start];
	assert(end_link.rel_vert < remote.nvert);

	if(start_link.abs_vert == end_link.abs_vert &&
	   end_link.patch == i) { //found the match
	  assert(!found);
	  nexus.borders[k + local.border_start] = Link(start_link.rel_vert, 
						      end_link.rel_vert, 
						      start_link.patch);
	  found = true;
	}
      }
      assert(nexus.borders[k + local.border_start].start_vert < local.nvert);
      assert(found);
    }
  }
  nexus.borders.Flush();

  //Checking border consistency:
  /*  for(unsigned int i = 0; i < nexus.index.size(); i++) {
    Border border = nexus.GetBorder(i);
    Nexus::Entry &entry = nexus.index[i];
    for(unsigned int k = 0; k < border.Size(); k++) {
      Link &link = border[k];
      if(link.start_vert >= entry.nvert) {
	cerr << "K: " << k << endl;
	cerr << "patch: " << i << " nvert: " << entry.nvert << " startv: " 
	     << link.start_vert << endl;
	cerr << "bstart: " << entry.border_start 
	     << "bsize: " << entry.border_size << endl;
      }
      assert(link.end_patch < nexus.index.size());
      assert(link.start_vert < entry.nvert);
      Nexus::Entry &remote = nexus.index[link.end_patch];
      assert(link.end_vert < remote.nvert);
    }
    
    }*/
}
