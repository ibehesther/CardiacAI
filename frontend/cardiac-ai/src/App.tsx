import React from "react";
import "./App.css";
import Header from "./components/header";
import { Route, Routes } from "react-router-dom";
import {routes } from "./routes/baseRoutes";
import { useAuth } from "./context/AuthProvider";

function App() {
	return (
		<div className="App">
			<Header />
			<Routes>
				{routes.map((route, index) => (
					<Route key={index} path={route.path} element={route.element} />
				))}
=			</Routes>
		</div>
	);
}

export default App;
