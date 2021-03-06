#pragma once

#include "Core/Debug.hpp"

#include <mtl/mtl.hpp>

#include <utl/memory.hpp>
#include <utl/structure_of_arrays.hpp>
#include <utl/functional.hpp>
#include <utl/vector.hpp>
#include <utl/concepts.hpp>
#include <utl/UUID.hpp>
#include <utl/messenger.hpp>

#include <span>
#include <mutex>
#include <optional>
#include <string>
#include <array>
#include <atomic>

#include "Core/BuildSystemFwd.hpp"
#include "Core/PluginManagerFwd.hpp"
#include "SharedNetworkTypes.hpp"
#include "Node.hpp"
#include "Pin.hpp"
#include "Edge.hpp"
#include "Node.hpp"

namespace worldmachine {
	
	class NodeImplementation;
	
	/// MARK: - EdgeCollection
	class EdgeCollection {
	private:
		using EdgeContainerType = utl::structure_of_arrays<Edge>;
		
	public:
		auto const& edgeParams() const { return m_edgeParams; }
		std::size_t edgeCount() const { return edges.size(); }
		
		EdgeContainerType edges;
		
	private:
		EdgeParameters m_edgeParams = defaultEdgeParameters();
	};
	
	
	/// MARK: - NodeCollection
	class NodeCollection {
	private:
		using NodeContainerType = utl::structure_of_arrays<Node>;

	public:
		NodeCollection();
		
		std::size_t addNode(NodeDescriptor);
		
		std::size_t nodeCount() const { return nodes.size(); }
		
		auto const& nodeParams() const { return m_nodeParams; }
		
	public:
		static mtl::rectangle<float> nodeBounds(mtl::float2 position, mtl::float2 size) {
			return { position - size / 2, size };
		}
		
		static mtl::float2 pinPosition(mtl::float2 nodePosition,
									   mtl::float2 nodeSize,
									   PinKind pinKind,
									   std::size_t pinIndex,
									   NodeParameters const& params);
		
		static mtl::disk<> pinDisk(mtl::float2 nodePosition,
								   mtl::float2 nodeSize,
								   PinKind,
								   std::size_t pinIndex,
								   NodeParameters const& params);
		
		NodeContainerType nodes;
		
	private:
		NodeParameters m_nodeParams = defaultNodeParameters();
	};
	
	/// MARK: - SelectionManager
	class SelectionManager {
	public:
		void clear() {
			indices.clear();
		}
		
		void clearCandidates() {
			candidates.clear();
		}
		
		void add(std::size_t index) {
			WM_Expect(audit, !isSelected(index), "index already present");
			indices.push_back(index);
		}
		
		void addCandidate(std::size_t index) {
			WM_Expect(audit, !isCandidate(index), "index already a candidate");
			candidates.push_back(index);
		}
		
		void remove(std::size_t index) {
			WM_Expect(audit, isSelected(index), "index not present");
			auto const result = std::find(indices.begin(), indices.end(), index);
			indices.erase(result);
		}
		
		bool tryRemove(std::size_t index) {
			auto const result = std::find(indices.begin(), indices.end(), index);
			if (result != indices.end()) {
				indices.erase(result);
				return true;
			}
			else {
				return false;
			}
		}
		
		bool isSelected(std::size_t index) const {
			return std::find(indices.begin(), indices.end(), index) != indices.end();
		}
		
		bool isCandidate(std::size_t index) const {
			return std::find(candidates.begin(), candidates.end(), index) != candidates.end();
		}
		
		utl::small_vector<std::size_t, 16> indices;
		utl::small_vector<std::size_t, 16> candidates;
	};
	
	/// MARK: - NetworkBase
	class NetworkBase:
		public NodeCollection,
		public EdgeCollection,
		protected SelectionManager
	{
	public:
		using NodeCollection::nodeCount;
		using EdgeCollection::edgeCount;
		using NodeCollection::nodeParams;
		using EdgeCollection::edgeParams;
		
		using SelectionManager::isSelected;
		
		void clear() { nodes.clear(); edges.clear(); }
		
		long indexFromID(utl::UUID id) const;
		utl::UUID IDFromIndex(std::size_t nodeIndex) const;
		utl::small_vector<std::size_t> indicesFromIDs(std::span<utl::UUID const> ids) const;
		utl::small_vector<utl::UUID> IDsFromIndices(std::span<std::size_t const> nodeIndices) const;
	
		std::span<std::size_t const> selectedIndices() const { return this->SelectionManager::indices; }
		
		[[nodiscard]] std::unique_lock<std::mutex> lock() {
			return std::unique_lock(_mutex);
		}
		auto locked(utl::invocable auto&& f) {
			std::lock_guard lock(_mutex);
			return f();
		}
		
		BuildInfo const& buildInfo() const { return _buildInfo; }
		bool isBuilding() const { return buildInfo().isBuilding(); }
		
	protected:
		bool testNodeFlag(std::size_t nodeIndex, NodeFlags flag) const {
			return !!(nodes[nodeIndex].flags & flag);
		}
		
		void setNodeFlag(std::size_t nodeIndex, NodeFlags flag) {
			nodes[nodeIndex].flags |= flag;
		}
		
		void clearNodeFlag(std::size_t nodeIndex, NodeFlags flag) {
			nodes[nodeIndex].flags &= ~flag;
		}
		
		void toggleNodeFlag(std::size_t nodeIndex, NodeFlags flag) {
			nodes[nodeIndex].flags ^= flag;
		}
		
		using Nodes     = NodeCollection;
		using Edges     = EdgeCollection;
		using Selection = SelectionManager;
		
		std::mutex _mutex;
		
		friend class BuildSystem;
		BuildInfo _buildInfo;
		
		utl::vector<std::pair<ImplementationID, std::string>> _storedImplementationState;
		utl::listener_id_bag _listenerIDs;
	};
	
	/// MARK: - HitResult
	struct NetworkHitResult {
		enum struct Type {
			background, node, pin, edge
		};
		
		struct Pin {
			mtl::float2 position;
			std::size_t index;
			PinKind kind;
		};
		
		struct Node {
			std::size_t index;
			NodeCategory category;
		};
		
		struct Edge {
			std::size_t index;
		};
		
		Type type;
		
		union {
			struct {
				Pin pin;
				Node node;
			};
			Edge edge;
		};
	};
	
	enum struct SelectOperation {
		setUnion,
		setDifference,
		setIntersection,
		setSymmetricDifference
	};
	
	struct NetworkCycleError: std::runtime_error {
		NetworkCycleError(): NetworkCycleError("NetworkCycleError") {}
		NetworkCycleError(std::string msg): std::runtime_error(std::move(msg)) {}
		NetworkCycleError(char const* msg): std::runtime_error(msg) {}
	};
	
	struct NodeEdges {
		struct EdgeRepresentation: Edge {
			bool present = false;
			bool mandatory = true;
		};
		
		utl::small_vector<EdgeRepresentation, 4> inputEdges;
		utl::small_vector<EdgeRepresentation, 2> maskInputEdges;
		
		bool dependenciesConnected() const {
			return dependenciesConnectedImpl(inputEdges) && dependenciesConnectedImpl(maskInputEdges);
		}
		
	private:
		static bool dependenciesConnectedImpl(std::span<EdgeRepresentation const> ec) {
			for (EdgeRepresentation const& edge : ec) {
			   if (!edge.present && edge.mandatory) {
				   return false;
			   }
		   }
		   return true;
	   }
	};
	
	/// MARK: - Network
	class Network:
		public NetworkBase
	{
		Network(); // private
		
	public:
		static utl::unique_ref<Network> create();
		
		/// Modification and interaction
		NetworkHitResult hitTest(mtl::float2 hitPositionWS) const;
		
		void addEdge(PinIndex const&, PinIndex const&);
		void tryAddEdge(PinIndex const&, PinIndex const&);
		void removeEdge(std::size_t edgeIndex);
		
		void removeSelectedNodes();
		void removeNode(utl::UUID nodeID);
		void removeNode(std::size_t nodeIndex);
		
		void selectNode(std::size_t nodeIndex);
		void unselectNode(utl::UUID nodeID);
		void unselectNode(std::size_t nodeIndex);
		bool toggleSelectNode(std::size_t nodeIndex); // -> true iff node is selected after call
		void clearSelection();
		
		void setRectangleSelection(mtl::rectangle<float>, SelectOperation);
		void applyRectangleSelection(SelectOperation);
		
		void moveToTop(std::size_t nodeIndex); 
		void moveSelected(mtl::float2 offset);
		
		/// Queries
		utl::small_vector<std::size_t>  gatherLeafNodes();
		utl::small_vector<std::size_t>  gatherRootNodes();
		bool allMandatoryUpstreamNodesConnected(utl::UUID nodeID) const;
		bool allMandatoryUpstreamNodesConnected(std::size_t nodeIndex) const;
		
		void invalidateNodesDownstream(std::size_t nodeIndex, BuildType = BuildType::all);
		void invalidateNodesDownstream(utl::UUID nodeID, BuildType = BuildType::all);
		void invalidateAllNodes(BuildType = BuildType::all);
		
		NodeEdges collectNodeEdges(std::size_t nodeIndex) const;
		
	private:
		std::optional<NetworkHitResult::Pin> pinHitTest(mtl::float2 hitPosition,
																	mtl::float2 nodePosition,
																	mtl::float2 nodeSize,
																	PinCount<>) const;
		
		void addEdgeImpl(PinIndex const& from, PinIndex const& to,
						 std::optional<std::size_t> edgeToRemove = std::nullopt);
		EdgeProxy makeEdgeProxy(PinIndex const& from, PinIndex const& to);
		
		std::optional<std::size_t> edgeInPin(PinIndex const&) const;
		
		utl::small_vector<std::size_t> gatherNodesImpl(auto&& cond);
		
		template <bool Reverse>
		static void _traverseUpstreamNodesStartImpl(auto* _this, std::size_t nodeIndex, utl::invocable<std::size_t> auto&& f);
		template <bool Reverse>
		static void _traverseUpstreamNodesImpl(auto* _this, std::size_t nodeIndex, utl::invocable<std::size_t> auto&& f);
		
		template <bool Reverse>
		static void _traverseDownstreamNodesStartImpl(auto* _this, std::size_t nodeIndex, utl::invocable<std::size_t> auto&& f);
		template <bool Reverse>
		static void _traverseDownstreamNodesImpl(auto* _this, std::size_t nodeIndex, utl::invocable<std::size_t> auto&& f);
	};

	// MARK: - inline implementation
	namespace internal {
		class CycleChecker {
		public:
			CycleChecker(std::size_t beginNodeIndex): beginNodeIndex(beginNodeIndex), thisVisitCount{ 0 } {}
			
			void operator()(std::size_t nodeIndex) {
				if (nodeIndex == beginNodeIndex && thisVisitCount++ != 0) {
					throw NetworkCycleError();
				}
			}
			
		private:
			std::size_t beginNodeIndex, thisVisitCount;
		};
	}
	
}
