// Copyright (C) 2011-2012 by the BEM++ Authors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef gmsh_hpp
#define gmsh_hpp

#include <cstddef>
#include <vector>
#include <iostream>
#include <string>
#include <memory>
#include "../common/shared_ptr.hpp"

namespace Bempp {

class GmshData {

public:

    GmshData();
    int numberOfNodes() const;
    int numberOfElements() const;
    int numberOfPeriodicEntities() const;
    int numberOfPeriodicNodes() const;
    int numberOfPhysicalNames() const;
    int numberOfNodeDataSets() const;
    int numberOfElementDataSets() const;
    int numberOfElementNodeDataSets() const;
    int numberOfInterpolationSchemeSets() const;

    void addNode(int index, double x, double y, double z);
    void addElement(int index, int elementType, const std::vector<int>& nodes,
                    int physicalEntity, int elementaryEntity,
                    const std::vector<int>& partitions = std::vector<int>());

    void addPeriodicEntity(int dimension, int slaveEntityTag, int masterEntityTag);
    void addPeriodicNode(int slaveNode, int masterNode);

    void addPhysicalName(int dimension, int number, std::string name);

    void addNodeDataSet(const std::vector<std::string>& stringTags, const std::vector<double>& realTags,
                       int numberOfFieldComponents, int capacity=0, int timeStep=0, int partition=0);
    void addElementDataSet(const std::vector<std::string>& stringTags, const std::vector<double>& realTags,
                        int numberOfFieldComponents, int capacity=0, int timeStep=0, int partition=0);
    void addElementNodeDataSet(const std::vector<std::string>& stringTags, const std::vector<double>& realTags,
                        int numberOfFieldComponents,
                        int capacity=0, int timeStep=0, int partition=0);

    void addNodeData(int dataSetIndex, int node, const std::vector<double>& values);
    void addElementData(int dataSetIndex, int element, const std::vector<double>& values);
    void addElementNodeData(int dataSetIndex, int element, const std::vector<std::vector<double> >& values);

    void addInterpolationSchemeSet(std::string name, int topology);
    void addInterpolationMatrix(int dataSetIndex, int nrows, int ncols, const std::vector<double>& values);

    void getNodeIndices(std::vector<int>& indices) const;
    void getElementIndices(std::vector<int>& indices) const;

    void getNode(int index, double &x, double &y, double &z) const;
    void getElement(int index, int& elementType, std::vector<int>& nodes,
                    int& physicalEntity, int& elementaryEntity, std::vector<int>& partitions) const;
    void getElement(int index, int& elementType, std::vector<int>& nodes,
                    int& physicalEntity, int& elementaryEntity) const;
    void getPeriodicEntity(int index, int& dimension, int& slaveEntityTag, int& masterEntityTag) const;
    void getPeriodicNode(int index, int& slaveNode, int& masterNode) const;
    void getPhysicalName(int index, int& dimension, int& number, std::string& name) const;
    void getNodeDataSet(int index, std::vector<std::string>& stringTags, std::vector<double>& realTags,
                        int& numberOfFieldComponents,
                        std::vector<int>& nodeIndices, std::vector<std::vector<double> >& values,
                        int& timeStep, int& partition) const;
    void getNodeDataSet(int index, std::vector<std::string>& stringTags, std::vector<double>& realTags,
                        int& numberOfFieldComponents,
                        std::vector<int>& nodeIndices, std::vector<std::vector<double> >& values);
    void getElementDataSet(int index, std::vector<std::string>& stringTags, std::vector<double>& realTags,
                        int& numberOfFieldComponents,
                        std::vector<int>& elementIndices, std::vector<std::vector<double> >& values,
                        int& timeStep, int& partition) const;
    void getElementDataSet(int index, std::vector<std::string>& stringTags, std::vector<double>& realTags,
                        int& numberOfFieldComponents,
                        std::vector<int>& elementIndices, std::vector<std::vector<double> >& values) const;
    void getElementNodeDataSet(int index, std::vector<std::string>& stringTags, std::vector<double>& realTags,
                        int& numberOfFieldComponents,
                        std::vector<int>& elementIndices, std::vector<std::vector<std::vector<double> > >& values,
                        int& timeStep, int& partition) const;
    void getElementNodeDataSet(int index, std::vector<std::string>& stringTags, std::vector<double>& realTags,
                        int& numberOfFieldComponents,
                        std::vector<int>& elementIndices, std::vector<std::vector<std::vector<double> > >& values) const;
    void getInterpolationSchemeSet(int index, std::string& name, int& topology, std::vector<int> &nrows,
                                   std::vector<int> &ncols,
                                   std::vector<std::vector<double> >& values);

    void reserveNumberOfNodes(int n);
    void reserveNumberOfElements(int n);

    void write(std::ostream& output) const;
    void write(const std::string& fname) const;
    static GmshData read(std::istream& input);
    static GmshData read(const std::string& fname);


private:

    struct NodeDataSet {


        std::vector<std::string> stringTags;
        std::vector<double> realTags;
        std::vector<int> integerTags;
        int timeStep;
        int numberOfFieldComponents;
        int partition;

        std::vector<int> nodeIndices;
        std::vector<std::vector<double> > values;

    };

    struct ElementDataSet {


        std::vector<std::string> stringTags;
        std::vector<double> realTags;
        std::vector<int> integerTags;
        int timeStep;
        int numberOfFieldComponents;
        int partition;

        std::vector<int> elementIndices;
        std::vector<std::vector<double> > values;

    };

    struct ElementNodeDataSet {


        std::vector<std::string> stringTags;
        std::vector<double> realTags;
        std::vector<int> integerTags;
        int timeStep;
        int numberOfFieldComponents;
        int partition;

        std::vector<int> elementIndices;
        std::vector<std::vector<std::vector<double> > > values;

    };


    struct InterpolationSchemeSet {

        std::vector<int> nrows;
        std::vector<int> ncols;
        std::vector<std::vector<double> > values; // Each outer array element stores
                                                  // one matrix in row-major order

        std::string name;
        int topology;

    };

    struct Node {
        double x;
        double y;
        double z;
    };

    struct Element {
        int type;
        int physicalEntity;
        int elementaryEntity;
        std::vector<int> nodes;
        std::vector<int> partitions;
    };

    struct PeriodicEntity {
        int dimension;
        int slaveTag;
        int masterTag;
    };

    struct PeriodicNode {
        int slaveNode;
        int masterNode;
    };

    struct PhysicalName {
        int dimension;
        int number;
        std::string name;
    };

    std::string m_versionNumber;
    int m_fileType;
    int m_dataSize;
    int m_numberOfNodes;
    int m_numberOfElements;

    std::vector<shared_ptr<Node> > m_nodes;
    std::vector<shared_ptr<Element> > m_elements;

    std::vector<PeriodicEntity> m_periodicEntities;
    std::vector<PeriodicNode> m_periodicNodes;

    std::vector<PhysicalName> m_physicalNames;

    std::vector<shared_ptr<NodeDataSet> > m_nodeDataSets;
    std::vector<shared_ptr<ElementDataSet> > m_elementDataSets;
    std::vector<shared_ptr<ElementNodeDataSet> > m_elementNodeDataSets;

    std::vector<shared_ptr<InterpolationSchemeSet> > m_interpolationSchemeSets;

};



} // namespace
#endif
