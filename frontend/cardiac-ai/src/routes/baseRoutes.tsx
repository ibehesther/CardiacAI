import React from "react";
import { lazy } from "react";
import Layout from "../layout";

const Home = lazy(() => import("../pages/home"));
const About = lazy(() => import("../pages/about"));
const Login = lazy(() => import("../pages/authentication/login"));
const Analysis = lazy(() => import("../pages/analysis"));


export const routes = [
	{
		path: "/",
		element: (
			<Layout>
				<Home />
			</Layout>
		),
	},
	{
		path: "/about",
		element: (
			<Layout>
				<About />
			</Layout>
		),
	},
	{
		path: "/login",
		element: (
			<Layout>
				<Login />
			</Layout>
		),
	},
	{
		path: "/analysis",
		element: (
			<Layout>
				<Analysis />
			</Layout>
		),
	},
];
