/*
 * Copyright 2015 Delft University of Technology
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package nl.tudelft.graphalytics.graphmat.algorithms.bfs;

import nl.tudelft.graphalytics.domain.algorithms.BreadthFirstSearchParameters;
import nl.tudelft.graphalytics.graphmat.GraphMatJob;

import org.apache.commons.configuration.Configuration;
import org.apache.commons.exec.CommandLine;

import it.unimi.dsi.fastutil.longs.Long2LongMap;

import java.nio.file.Paths;
import java.util.List;

/**
 * Breadth-first search job implementation for GraphMat. This class is responsible for formatting BFS-specific
 * arguments to be passed to the GraphMat executable, and does not include the implementation of the algorithm.
 *
 * @author Yong Guo
 * @author Tim Hegeman
 */
public final class BreadthFirstSearchJob extends GraphMatJob {

	private final BreadthFirstSearchParameters params;

	public BreadthFirstSearchJob(Configuration config, String graphPath, Long2LongMap vertexTranslation, BreadthFirstSearchParameters params) {
		super(config, graphPath, vertexTranslation);
		this.params = params;
	}

	@Override
	protected String getExecutable() {
		return "bfs";
	}

	@Override
	protected void addJobArguments(List<String> args) {
		long oldSource = params.getSourceVertex();
		long newSource = vertexTranslation.get(oldSource);
		
		args.add(Long.toString(newSource));
	}
}
