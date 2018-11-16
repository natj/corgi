from mpi4py import MPI

import unittest

import numpy as np
import sys
import pycorgi.twoD as pycorgi

#sys.path.append('../lib')


class Params:
    mins = None
    maxs = None
    lens = None


class Initialization(unittest.TestCase):
    Nx = 10
    Ny = 20

    xmin = 0.0
    xmax = 1.0
    ymin = 2.0
    ymax = 3.0

    def setUp(self):
        self.node = pycorgi.Node(self.Nx, self.Ny)

        self.node.setGridLims(self.xmin, self.xmax,
                              self.ymin, self.ymax
                              )

    def test_size(self):
        nx = self.node.getNx()
        ny = self.node.getNy()

        self.assertEqual(self.node.getNx(), self.Nx)
        self.assertEqual(self.node.getNy(), self.Ny)

    def test_physicalSize(self):
        self.assertEqual( self.node.getXmin(), self.xmin )
        self.assertEqual( self.node.getXmax(), self.xmax )

        self.assertEqual( self.node.getYmin(), self.ymin )
        self.assertEqual( self.node.getYmax(), self.ymax )


def tileID(i,j,Nx,Ny):
    return j*Nx + i


class Parallel(unittest.TestCase):
    
    Nx = 10
    Ny = 15

    xmin = 0.0
    xmax = 1.0
    ymin = 2.0
    ymax = 3.0


    def setUp(self):
        self.node = pycorgi.Node(self.Nx, self.Ny)
        self.node.setGridLims(self.xmin, self.xmax, self.ymin, self.ymax)

    def test_mpiInitialization(self):

        self.refGrid = np.zeros((self.Nx, self.Ny), np.int)
        self.refGrid[0:5,   0:10] = 0
        self.refGrid[0:5,  10:15] = 1
        self.refGrid[5:10,  0:10] = 2
        self.refGrid[5:10, 10:15] = 3

        if self.node.master():
            for j in range(self.node.getNy()):
                for i in range(self.node.getNx()):
                    val = self.refGrid[i,j]
                    self.node.setMpiGrid(i, j, val )
        self.node.bcastMpiGrid()

        for j in range(self.node.getNy()):
            for i in range(self.node.getNx()):
                val = self.node.getMpiGrid(i,j)
                self.assertEqual(val, self.refGrid[i,j])

    def test_cid(self):
        for j in range(self.node.getNy()):
            for i in range(self.node.getNx()):
                cid = self.node.id(i, j)
                cidr = tileID( i, j, self.node.getNx(), self.node.getNy() )
                self.assertEqual(cid, cidr)

    def test_loading(self):

        #load tiles
        k = 0
        for j in range(self.node.getNy()):
            for i in range(self.node.getNx()):
                c = pycorgi.Tile()
                self.node.addTile(c, (i,j) ) 
                k += 1
        self.assertEqual( k, self.Nx*self.Ny )

        cids = self.node.getTileIds() 
        self.assertEqual( len(cids), self.Nx*self.Ny )

        #now try and get then back
        for cid in cids:
            c = self.node.getTile(cid)

            self.assertEqual(c.cid,   cid)
            self.assertEqual(c.communication.owner, self.node.rank())
            self.assertEqual(c.communication.local, True)

            #ci = c.i
            #cj = c.j
            #self.assertEqual(ci, ri)
            #self.assertEqual(cj, rj)


# advanced parallel tests
class Parallel2(unittest.TestCase):
    
    Nx = 10
    Ny = 2

    xmin = 0.0
    xmax = 1.0
    ymin = 2.0
    ymax = 3.0


    def test_send_recv(self):

        #set up
        node = pycorgi.Node(self.Nx, self.Ny)
        node.setGridLims(self.xmin, self.xmax, self.ymin, self.ymax)

        # divide into upper and lower halfs
        #refGrid = np.zeros((self.Nx, self.Ny), np.int)
        #refGrid[:, 0] = 0
        #refGrid[:, 1] = 1

        #if node.master():
        #    for j in range(node.getNy()):
        #        for i in range(node.getNx()):
        #            val = refGrid[i,j]
        #            node.setMpiGrid(i, j, val )
        #node.bcastMpiGrid()

        #load tiles
        if node.rank() == 0:
            for j in range(node.getNy()):
                for i in range(node.getNx()):
                    #if node.getMpiGrid(i,j) == 0:
                    c = pycorgi.Tile()
                    node.addTile(c, (i,j) ) 

        #0 sends
        if node.rank() == 0 and node.size() > 1:
            #load cell with info
            cid = node.id(2, 1)
            #print("0:  send............cid:", cid)
            c = node.getTile(cid)

            #communication object
            c.communication.top_virtual_owner           = 10
            c.communication.communications              = 11
            c.communication.number_of_virtual_neighbors = 12

            c.set_tile_mins([1.0,2.0])
            c.set_tile_maxs([1.1,2.1])



            #get the same cell and send
            node.send_tile(cid, 1)


        #1 does nothing but receives
        if node.rank() == 1 and node.size() > 1:
            node.recv_tile(0)
            #print("1 recv............")
        
        #node.wait()

        #assert that we received the tile properly
        if node.rank() == 1 and node.size() > 1:
            cid = node.id(2, 1)
            c = node.getTile(cid)
            #print("1:  cid=", cid)

            #cid
            #owner
            #top_virtual_owner
            #communications
            #number_of_virtual_neighbors

            self.assertEqual(c.communication.cid, cid)
            self.assertEqual(c.communication.owner, 0)
            self.assertEqual(c.communication.top_virtual_owner, 10)
            self.assertEqual(c.communication.communications,    11)
            self.assertEqual(c.communication.number_of_virtual_neighbors, 12)

            #indices
            #mins
            #maxs
            self.assertEqual(c.communication.indices, [2, 1, 0] )
            self.assertEqual(c.communication.mins,    [1.0, 2.0, 0.0] )
            self.assertEqual(c.communication.maxs,    [1.1, 2.1, 0.0] )

            #tile variables
            self.assertEqual(c.cid,     cid)
            self.assertEqual(c.mins,    [1.0, 2.0] )
            self.assertEqual(c.maxs,    [1.1, 2.1] )
            self.assertEqual(c.index,   (2,1) )



            #check that cell is reconstructed correctly from Communication obj
            self.assertEqual(c.cid, cid)


if __name__ == '__main__':
    unittest.main()


