#pragma once

#include <utl/vector.hpp>
#include <utl/functional.hpp>

namespace worldmachine {
	
	class BuildJob {
		friend class BuildSystem;
	public:
		void add(utl::function<void()> f) {
			jobs.push_back(std::move(f));
		}
		
		void reserve(std::size_t size) {
			jobs.reserve(size);
		}
		
		void onCompletion(utl::function<void()> f) {
			completionHandler = std::move(f);
		}
		void onFailure(utl::function<void()> f) {
			failureHandler = std::move(f);
		}
		void onCleanup(utl::function<void()> f) {
			cleanupHandler = std::move(f);
		}
		
	private:
		bool hasJobs() const { return index < jobs.size(); }
		utl::function<void()> consumeOne() {
			return std::move(jobs[index++]);
		}
		
		float oneProgress() const { return 1.0f / jobs.size(); }
		
		utl::vector<utl::function<void()>> jobs;
		utl::function<void()> completionHandler, failureHandler, cleanupHandler;
		std::size_t index = 0;
	};
	
}
